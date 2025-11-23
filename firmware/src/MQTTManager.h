#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "NetManager.h"
#include "LTEModule.h"


class MQTTManager {
public:
    using MsgCallback = std::function<void(String topic, String payload)>;

    MQTTManager(const char* host, int port);
    void begin(MsgCallback cb);
    void loop();
    void reconnect(NetworkType netType);

private:
    WiFiClient _wifiClient;
    PubSubClient _mqtt;

    const char* _host;
    int _port;

    MsgCallback _userCb;
    NetworkType _currentNet = NET_NONE;
    uint32_t _lastRetry = 0;



    bool connectViaWiFi();
    bool connectVia4G();

    void onWiFiMessage(char* topic, byte* payload, unsigned int length);
};
