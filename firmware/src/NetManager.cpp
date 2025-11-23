#include "NetManager.h"
#include <Preferences.h>
#include <BLE2902.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_SSID "12345678-1234-1234-1234-1234567890ac"
#define CHARACTERISTIC_PWD "12345678-1234-1234-1234-1234567890ad"
#define CHARACTERISTIC_STATUS "12345678-1234-1234-1234-1234567890ae"
#define CHARACTERISTIC_SCAN_REQ "12345678-1234-1234-1234-1234567890af"
#define CHARACTERISTIC_WIFI_LIST "12345678-1234-1234-1234-1234567890b0"

// ---------- BLE 回调类 ----------
class WiFiInfoCallback : public BLECharacteristicCallbacks
{
public:
    WiFiInfoCallback(NetworkManager *mgr) : _mgr(mgr) {}

private:
    NetworkManager *_mgr;
    void onWrite(BLECharacteristic *ch) override
    {

        if (!_mgr->bleProvisionActive)
        {
            return;
        }

        if (_mgr->_currentNet == NET_WIFI)
        {
            Serial.println("⛔ 已联网，不接收配网请求");
            return;
        }

        std::string val = ch->getValue();
        if (ch->getUUID().toString() == CHARACTERISTIC_SSID)
        {
            _mgr->_ssid = strdup(val.c_str());
            Serial.println("📥 新 SSID: " + String(_mgr->_ssid));
        }
        if (ch->getUUID().toString() == CHARACTERISTIC_PWD)
        {
            _mgr->_pwd = strdup(val.c_str());
            Serial.println("📥 新 Password");
        }

        if (_mgr->_ssid && _mgr->_pwd)
        {
            Serial.println("🚀 已获取凭据，尝试连接 WiFi");
            _mgr->begin(_mgr->_ssid, _mgr->_pwd);
        }
    }
};

// WiFi 扫描回调
class ScanReqCallback : public BLECharacteristicCallbacks
{
public:
    ScanReqCallback(NetworkManager *mgr) : _mgr(mgr) {}

private:
    NetworkManager *_mgr;
    void onWrite(BLECharacteristic *) override
    {
        Serial.println("📡 App 请求 WiFi 扫描");
        _mgr->scanWifiList();
    }
};

// ---------- NetworkManager 实现 ----------
NetworkManager::NetworkManager()
{
    _lastCheck = millis();
    _lastProvisionScan = millis();
}

void NetworkManager::beginFromNVS()
{
    // ⚠️ 覆盖前先释放旧内存，避免内存泄漏
    if (_ssid)
        free((void *)_ssid);
    if (_pwd)
        free((void *)_pwd);

    Preferences pref;
    pref.begin("network", true);
    String ssid = pref.getString("ssid", "");
    String pwd = pref.getString("pwd", "");
    pref.end();

    if (ssid.isEmpty())
    {
        Serial.println("⚠️ NVS 没有 WiFi 配置，开启 BLE 配网");
        startBLEProvisioning();
        bleProvisionActive = true;
        return;
    }

    if (!ssid.isEmpty())
    {
        _ssid = strdup(ssid.c_str());
    }
    if (!pwd.isEmpty())
    {
        _pwd = strdup(pwd.c_str());
    }

    Serial.printf("🔄 加载NVS SSID: %s, PWD: ******\n", _ssid);

    begin(_ssid, _pwd);
}

void NetworkManager::begin(const char *ssid, const char *pwd)
{
    _ssid = ssid;
    _pwd = pwd;
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(_ssid, _pwd);
    Serial.println("📶 WiFi Connecting...");
    notifyStatus(CONNECTING);
}

void NetworkManager::loop()
{
    checkNetwork();

    if (!bleAssistActive)
        return;

    if (millis() - _lastProvisionScan > 10000)
    {
        _lastProvisionScan = millis();
        scanForProvisioning();
    }
}

void NetworkManager::checkNetwork()
{
    wl_status_t s = WiFi.status();

    if (s == _lastStatus)
        return;

    _lastStatus = s;

    switch (s)
    {
    case WL_CONNECTED:
        _currentNet = NET_WIFI;
        if (!credentialsSaved)
        {
            credentialsSaved = true;
            saveCredentials();
            notifyStatus(SUCCESS);
            Serial.println("🟢 WiFi 已连接，保存参数！");
        }

        if (!bleAssistActive)
        {
            Serial.println("🔋 禁用 BLE 配网 → 启用辅助配网模式");
            // BLEDevice::deinit(true);
            bleProvisionActive = false;
            // BLEDevice::init("");
            bleAssistActive = true;
        }
        break;

    case WL_CONNECT_FAILED:
        _currentNet = NET_NONE;
        notifyStatus(ERR_AUTH_FAIL);
        credentialsSaved = false;
        Serial.println("❌ 密码错误");
        break;

    case WL_NO_SSID_AVAIL:
        _currentNet = NET_NONE;
        notifyStatus(ERR_NO_AP_FOUND);
        credentialsSaved = false;
        // print ssid for debug
        Serial.println("🚫 SSID: " + String(_ssid));
        Serial.println("🚫 找不到 AP");
        break;

    default:
        _currentNet = NET_NONE;
        notifyStatus(CONNECTING);
        break;
    }

    if (_currentNet != _lastNet)
    {
        _lastNet = _currentNet;
        if (_callback)
            _callback(_currentNet);
    }

    if (_currentNet == NET_NONE && !bleProvisionActive)
    {
        Serial.println("📡 WiFi 无网络 → 启动 BLE 配网模式");
        // startBLEProvisioning();
        bleProvisionActive = true;
        bleAssistActive = false;
    }
}

void NetworkManager::startBLEProvisioning(String deviceName)
{
    Serial.println("📡 BLE 配网模式启动");

    BLEDevice::init(deviceName.c_str());
    BLEServer *server = BLEDevice::createServer();
    BLEService *service = server->createService(SERVICE_UUID);

    auto ssidCh = service->createCharacteristic(CHARACTERISTIC_SSID,
                                                BLECharacteristic::PROPERTY_WRITE);
    auto pwdCh = service->createCharacteristic(CHARACTERISTIC_PWD,
                                               BLECharacteristic::PROPERTY_WRITE);

    ssidCh->setCallbacks(new WiFiInfoCallback(this));
    pwdCh->setCallbacks(new WiFiInfoCallback(this));

    statusChar = service->createCharacteristic(
        CHARACTERISTIC_STATUS,
        BLECharacteristic::PROPERTY_NOTIFY);
    statusChar->addDescriptor(new BLE2902());

    scanReqChar = service->createCharacteristic(
        CHARACTERISTIC_SCAN_REQ,
        BLECharacteristic::PROPERTY_WRITE);
    scanReqChar->setCallbacks(new ScanReqCallback(this));

    wifiListChar = service->createCharacteristic(
        CHARACTERISTIC_WIFI_LIST,
        BLECharacteristic::PROPERTY_NOTIFY);
    wifiListChar->addDescriptor(new BLE2902());

    service->start();

    BLEAdvertising *advertising = server->getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();
}

void NetworkManager::notifyStatus(uint8_t state)
{
    if (statusChar)
    {
        statusChar->setValue(&state, 1);
        statusChar->notify();
    }
}

void NetworkManager::saveCredentials()
{
    Preferences pref;
    pref.begin("network", false);
    pref.putString("ssid", _ssid);
    pref.putString("pwd", _pwd);
    pref.end();
    Serial.println("💾 WiFi 配置已保存");
}

void NetworkManager::clearCredentials()
{
    Preferences pref;
    pref.begin("network", false);
    pref.clear();
    pref.end();
    Serial.println("🗑️ WiFi 配置已清除");
}

void NetworkManager::scanWifiList()
{
    // 若重新扫描，重置保存状态
    credentialsSaved = false;

    Serial.println("📡 Scanning WiFi...");

    int n = WiFi.scanNetworks();
    if (n <= 0)
        return;

    // Map 用于去重并保留最优信号
    std::map<String, int> wifiMap;

    for (int i = 0; i < n; i++)
    {
        String ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);

        if (ssid.length() == 0)
            continue;

        if (!wifiMap.count(ssid) || rssi > wifiMap[ssid])
        {
            wifiMap[ssid] = rssi; // 只保留信号最强值
        }
    }

    // 逐条 Notify 给手机
    for (auto &w : wifiMap)
    {
        String packet = w.first + "," + String(w.second);
        Serial.println("📤 " + packet);

        wifiListChar->setValue(packet.c_str());
        wifiListChar->notify();
        delay(50);
    }

    Serial.printf("📶 WiFi列表发送完毕，共 %d 条\n", wifiMap.size());
}

void NetworkManager::set4GChecker(std::function<bool()> checker) { _check4G = checker; }
void NetworkManager::setCallback(NetCallback cb) { _callback = cb; }

void NetworkManager::scanForProvisioning()
{
    // Placeholder for scanning BLE devices for provisioning

    // 若当前没有网络 → 不给别人配网
    if (_currentNet == NET_NONE)
        return;

    Serial.println("📡 网络正常，开始扫描其他设备用于配网...");

    // BLEDevice::init("");
    BLEScan *scan = BLEDevice::getScan();
    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(50);

    BLEScanResults results = scan->start(5, false);

    int count = results.getCount();
    Serial.printf("🔍 扫描到 %d 个 BLE 广播设备\n", count);

    for (int i = 0; i < count; i++)
    {
        BLEAdvertisedDevice dev = results.getDevice(i);

        if (dev.haveServiceUUID() &&
            dev.isAdvertisingService(BLEUUID(SERVICE_UUID)))
        {
            Serial.println("✨ 找到待配网设备！尝试连接...");

            // ⚡ 启动给对方配网流程
            provisionOtherDevice(dev);

            break;
        }
    }

    scan->clearResults();
    Serial.println("🛑 扫描结束");
}
void NetworkManager::provisionOtherDevice(BLEAdvertisedDevice dev)
{
    Serial.println("🔗 连接设备中...");

    BLEClient *client = BLEDevice::createClient();
    if (!client->connect(&dev))
    {
        Serial.println("❌ 连接失败");
        return;
    }
    Serial.println("🔗 已连接");

    BLERemoteService *service =
        client->getService(BLEUUID(SERVICE_UUID));

    if (!service)
    {
        Serial.println("❌ 无配网服务");
        client->disconnect();
        return;
    }

    BLERemoteCharacteristic *ch_ssid = service->getCharacteristic(CHARACTERISTIC_SSID);
    BLERemoteCharacteristic *ch_pwd = service->getCharacteristic(CHARACTERISTIC_PWD);

    if (!ch_ssid || !ch_pwd)
    {
        Serial.println("❌ 未找到配网特征值");
        client->disconnect();
        return;
    }

    // ⭐ 从 NVS 获取当前 WiFi 信息
    Preferences prefs;
    prefs.begin("network", true);
    String ssid = prefs.getString("ssid");
    String pwd = prefs.getString("pwd");
    prefs.end();

    Serial.println("📤 发送 SSID/PWD 给对方设备...");
    ch_ssid->writeValue(ssid.c_str());
    delay(50);
    ch_pwd->writeValue(pwd.c_str());

    Serial.println("🎯 配网指令已发送");
    client->disconnect();
}
