#pragma once
#include <WiFi.h>
#include <functional>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

enum NetworkType {
    NET_NONE = 0,
    NET_WIFI,
    NET_4G
};

enum WiFiProvStatus {
    CONNECTING = 3,
    SUCCESS    = 4,
    ERR_NO_AP_FOUND = 5,
    ERR_AUTH_FAIL   = 6
};

class NetworkManager {
public:
    using NetCallback = std::function<void(NetworkType)>;

    NetworkManager();
    void beginFromNVS();
    void begin(const char* ssid, const char* pwd);
    void loop();

    void set4GChecker(std::function<bool()> checker);
    void setCallback(NetCallback cb);

    void startBLEProvisioning(String deviceName = "ESP32-Provisioning");
    void scanWifiList();
    void notifyStatus(uint8_t state);
    void saveCredentials();
    void clearCredentials();

    NetworkType _currentNet = NET_NONE;
    NetworkType _lastNet    = NET_NONE;
    
    const char* _ssid = nullptr;
    const char* _pwd  = nullptr;
    
    bool credentialsSaved = false;
    bool bleProvisionActive = false;
    bool bleAssistActive = false;
private:

    unsigned long _lastCheck = 0;
    unsigned long _lastProvisionScan = 0;

    std::function<bool()> _check4G = nullptr;
    NetCallback _callback = nullptr;
        wl_status_t _lastStatus = WL_DISCONNECTED;


    BLECharacteristic* statusChar = nullptr;
    BLECharacteristic* scanReqChar = nullptr;
    BLECharacteristic* wifiListChar = nullptr;

    void checkNetwork();
    void scanForProvisioning();
    void provisionOtherDevice(BLEAdvertisedDevice dev);
};
