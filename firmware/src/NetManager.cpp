#include "NetManager.h"
#include <Preferences.h>
#include <BLE2902.h>
#include "ScreenLog.h"

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
            // // ScreenLog::instance().pushLog("bleProvisionActive false, ignoring WiFi info");
            return;
        }

        if (_mgr->_currentNet == NET_WIFI)
        {
            // // ScreenLog::instance().pushLog("Already connected to WiFi, ignoring new credentials");
            return;
        }

        std::string val = ch->getValue();
        if (ch->getUUID().toString() == CHARACTERISTIC_SSID)
        {
            _mgr->_ssid = strdup(val.c_str());
            // // ScreenLog::instance().pushLog("New SSID: " + String(_mgr->_ssid));
        }
        if (ch->getUUID().toString() == CHARACTERISTIC_PWD)
        {
            _mgr->_pwd = strdup(val.c_str());
            // // ScreenLog::instance().pushLog("New Password");
        }

        if (_mgr->_ssid && _mgr->_pwd)
        {
            // // ScreenLog::instance().pushLog("attempting to connect to WiFi");
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
        // // ScreenLog::instance().pushLog("WiFi scan requested by app");
        _mgr->scanWifiList();
    }
};

// BLE 扫描结果回调
class ScanResultCallback : public BLEAdvertisedDeviceCallbacks
{
public:
    ScanResultCallback(NetworkManager *mgr) : _mgr(mgr) {}

private:
    NetworkManager *_mgr;
    void onResult(BLEAdvertisedDevice advertisedDevice) override
    {

        if (advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID)))
        {
            // // ScreenLog::instance().pushLog("device found +");
            _mgr->provisionOtherDevice(advertisedDevice);
        }

        // ⭐ 发现目标设备后立即停止扫描
        // BLEDevice::getScan()->stop();
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
        Serial.println("No saved SSID, starting BLE provisioning");
        // // ScreenLog::instance().pushLog("no saved SSID, starting BLE provisioning");
        // setupBLEProvisioning();
        // startAdvertising();
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

    // // ScreenLog::instance().pushLog("Loading NVS SSID: " + String(_ssid) + ", PWD: ******");

    Serial.printf("Connecting to WiFi SSID: %s\n", _ssid);

    begin(_ssid, _pwd);
}

void NetworkManager::begin(const char *ssid, const char *pwd)
{
    _ssid = ssid;
    _pwd = pwd;
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(_ssid, _pwd);
    // // ScreenLog::instance().pushLog("WiFi Connecting...");
    notifyStatus(CONNECTING);
}

void NetworkManager::loop()
{
    checkWifi();
    checkNetwork();

    // if (bleAssistActive)
    //     startAsyncScan();
    // scanForProvisioning();
}

void NetworkManager::checkWifi()
{
    wl_status_t s = WiFi.status();
    if (s == _lastStatus)
        return;

    _lastStatus = s;

    switch (s)
    {
    case WL_CONNECTED:
        if (!credentialsSaved)
        {
            credentialsSaved = true;
            saveCredentials();
            notifyStatus(SUCCESS);
            // // ScreenLog::instance().pushLog("WiFi connected, credentials saved.");
        }

        // WiFi 连接后关闭 BLE 配网
        bleProvisionActive = false;
        bleAssistActive = true;
        stopAdvertising();
        break;

    case WL_CONNECT_FAILED:
        notifyStatus(ERR_AUTH_FAIL);
        credentialsSaved = false;
        // // ScreenLog::instance().pushLog("Password error");
        break;

    case WL_NO_SSID_AVAIL:
        notifyStatus(ERR_NO_AP_FOUND);
        credentialsSaved = false;
        // // ScreenLog::instance().pushLog("SSID Not Found: " + String(_ssid));
        break;

    default:
        notifyStatus(CONNECTING);
        break;
    }

    if (s != WL_CONNECTED && !bleProvisionActive)
    {
        // // ScreenLog::instance().pushLog("WiFi no network → Start BLE provisioning");

        startAdvertising();
        bleProvisionActive = true;
        bleAssistActive = false;
    }

    Serial.printf("WiFi status changed: %d\n", s);
}

void NetworkManager::checkNetwork()
{
    bool wifiOK = (WiFi.status() == WL_CONNECTED);
    bool lteOK  = _lteStatus;

    NetworkType newNet;

    if (wifiOK)
        newNet = NET_WIFI;
    else if (lteOK)
        newNet = NET_4G;
    else
        newNet = NET_NONE;


    if (newNet != _lastNet)
    {
        _lastNet = newNet;
        _currentNet = newNet;

        if (_callback)
            _callback(newNet);
    }
}

void NetworkManager::setupBLEProvisioning(String deviceName)
{
    // // ScreenLog::instance().pushLog("Starting BLE provisioning mode");

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
    // BLEDevice::startAdvertising();
}

void NetworkManager::startAdvertising()
{
    BLEDevice::startAdvertising();
    // // ScreenLog::instance().pushLog("BLE advertising started");
}

void NetworkManager::stopAdvertising()
{
    BLEDevice::stopAdvertising();
    // // ScreenLog::instance().pushLog("BLE advertising stopped");
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
    // // ScreenLog::instance().pushLog("WiFi credentials saved");
}

void NetworkManager::clearCredentials()
{
    Preferences pref;
    pref.begin("network", false);
    pref.clear();
    pref.end();
    // // ScreenLog::instance().pushLog("WiFi credentials cleared");
}

void NetworkManager::scanWifiList()
{
    // 若重新扫描，重置保存状态
    credentialsSaved = false;

    // // ScreenLog::instance().pushLog("Scanning WiFi...");

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
        // // ScreenLog::instance().pushLog("info: " + packet);

        wifiListChar->setValue(packet.c_str());
        wifiListChar->notify();
        delay(50);
    }

    // // ScreenLog::instance().pushLog("WiFi list sent, total " + String(wifiMap.size()) + " entries");
}

void NetworkManager::set4Gstatus(bool status) { _lteStatus = status; }

void NetworkManager::setCallback(NetCallback cb) { _callback = cb; }

void NetworkManager::scanForProvisioning()
{
    // Placeholder for scanning BLE devices for provisioning

    // 若当前没有网络 → 不给别人配网
    if (_currentNet == NET_NONE)
        return;

    // // ScreenLog::instance().pushLog("Network normal, scanning other devices for provisioning...");

    // BLEDevice::init("");
    BLEScan *scan = BLEDevice::getScan();
    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(50);

    BLEScanResults results = scan->start(5, false);

    int count = results.getCount();
    // // ScreenLog::instance().pushLog("Scanned " + String(count) + " BLE advertising devices");

    for (int i = 0; i < count; i++)
    {
        BLEAdvertisedDevice dev = results.getDevice(i);

        if (dev.haveServiceUUID() &&
            dev.isAdvertisingService(BLEUUID(SERVICE_UUID)))
        {
            // // ScreenLog::instance().pushLog("Found device for provisioning! Trying to connect...");

            // ⚡ 启动给对方配网流程
            provisionOtherDevice(dev);

            break;
        }
    }

    scan->clearResults();
    // Serial.println("🛑 扫描结束");
}
void NetworkManager::provisionOtherDevice(BLEAdvertisedDevice dev)
{
    if (!bleAssistActive || _currentNet != NET_WIFI)
    {
        /* code */
    }

    // // ScreenLog::instance().pushLog("Connecting to device...");
    BLEClient *client = BLEDevice::createClient();
    if (!client->connect(&dev))
    {
        // // ScreenLog::instance().pushLog("Connection failed");
        return;
    }
    // // ScreenLog::instance().pushLog("Connected");

    BLERemoteService *service =
        client->getService(BLEUUID(SERVICE_UUID));

    if (!service)
    {
        // // ScreenLog::instance().pushLog("No provisioning service found");
        client->disconnect();
        return;
    }

    BLERemoteCharacteristic *ch_ssid = service->getCharacteristic(CHARACTERISTIC_SSID);
    BLERemoteCharacteristic *ch_pwd = service->getCharacteristic(CHARACTERISTIC_PWD);

    if (!ch_ssid || !ch_pwd)
    {
        // // ScreenLog::instance().pushLog("No provisioning characteristics found");
        client->disconnect();
        return;
    }

    // // ScreenLog::instance().pushLog("Sending SSID/PWD to the device...");
    ch_ssid->writeValue(_ssid);
    delay(50);
    ch_pwd->writeValue(_pwd);

    // // ScreenLog::instance().pushLog("Provisioning command sent");
    client->disconnect();
}
