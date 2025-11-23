#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "NetManager.h"
#include "LTEModule.h"

class MQTTManager
{
public:
    MQTTManager();

    using MsgCallback = std::function<void(String topic, String payload)>;
    void init();
    void setupCallback(MsgCallback cb);

    void setLTEModule(LTEModule *modem);

    void connectViaWiFi();
    void connectVia4G();
    void loop();

    void onMessage(char *topic, byte *payload, unsigned int length);
    static void mqttCallback(char *topic, byte *payload, unsigned int length);

    void sendLog(String message);
    void sendDeviceInfo();

private:
    WiFiClient _wifiClient;
    PubSubClient _mqtt;
    LTEModule *_modem = nullptr;

    const char *_host;
    int _port;

    MsgCallback _userCb;
    NetworkType _currentNet = NET_NONE;
    uint32_t _lastRetry = 0;
};
