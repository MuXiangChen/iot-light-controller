#pragma once
#include <WiFi.h>
#include <functional>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

enum NetworkType
{
    NET_NONE = 0,
    NET_WIFI,
    NET_4G
};

enum WiFiProvStatus
{
    CONNECTING = 3,
    SUCCESS = 4,
    ERR_NO_AP_FOUND = 5,
    ERR_AUTH_FAIL = 6
};

enum ProvisionState
{
    IDLE,
    SCANNING,
    FOUND_DEVICE,
    BLECONNECTING,
    SENDING_SSID,
    SENDING_PWD,
    DONE
};

class NetworkManager
{
public:
    using NetCallback = std::function<void(NetworkType)>;

    String deviceID;

    NetworkManager();
    void beginFromNVS();
    void begin(const char *ssid, const char *pwd);
    void loop();

    void set4Gstatus(bool status);
    void setCallback(NetCallback cb);

    void setupBLEProvisioning(String deviceName = "ESP32-Provisioning");
    void startAdvertising();
    void stopAdvertising();

    void scanWifiList();
    void notifyStatus(uint8_t state);
    void saveCredentials();
    void clearCredentials();

    NetworkType _currentNet = NET_NONE;
    NetworkType _lastNet = NET_NONE;

    const char *_ssid = nullptr;
    const char *_pwd = nullptr;

    bool credentialsSaved = false;
    bool bleProvisionActive = false;
    bool bleAssistActive = false;

    ProvisionState _provState = IDLE;
    BLEAdvertisedDevice _targetDev;
    BLEClient *_provClient = nullptr;
    unsigned long _provTimer = 0;

    // void startAsyncScan();
    void scanForProvisioning();
    void provisionOtherDevice(BLEAdvertisedDevice dev);

private:
    unsigned long _lastCheck = 0;
    unsigned long _lastProvisionScan = 0;

    bool _lteStatus = false;
    NetCallback _callback = nullptr;
    wl_status_t _lastStatus = WL_DISCONNECTED;

    BLECharacteristic *statusChar = nullptr;
    BLECharacteristic *scanReqChar = nullptr;
    BLECharacteristic *wifiListChar = nullptr;

    // bool scanInProgress = false;

    void checkNetwork();
    void checkWifi();
    // void scanForProvisioning1();
};
