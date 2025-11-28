#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "NetManager.h"
#include <ArduinoJson.h>
#include <functional>

class MQTTManager
{
public:
    using MsgCallback = std::function<void(String topic, JsonDocument doc)>;

    // 传入 WiFiClient* 和 可选 LTE Client*
    MQTTManager(Client *wifiClient = nullptr, Client *lteClient = nullptr);

    String deviceID;

    void init();
    void setupCallback(MsgCallback cb);

    void selectWiFi(); // 切换到 WiFi
    void select4G();   // 切换到 4G

    void loop();
    void connectIfNeeded();

    bool connected() { return _mqtt.connected(); }

    // void onMessage(char *topic, byte *payload, unsigned int length);
    // static void mqttCallback(char *topic, byte *payload, unsigned int length);

    // 示例推送
    void sendLog(String message);
    void sendDeviceInfo();

private:
    Client *_wifi = nullptr;
    Client *_lte = nullptr;

    Client *_client = nullptr; // 当前使用的 client
    PubSubClient _mqtt;

    MsgCallback _userCb;
    const char *_host = nullptr;
    int _port = 1883;

    NetworkType _currentNet = NET_NONE;
    uint32_t _lastRetry = 0;

    const char *topicBootReport = "iotlight/device";
    const char *topicDeviceStatus = "iotlight/status";

    char topicControl[50];

    void handleMqttMessage(char *topic, byte *payload, unsigned int length);
};
