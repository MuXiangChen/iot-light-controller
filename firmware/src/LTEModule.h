#pragma once
#include <Arduino.h>
#include <functional>

class LTEModule {
public:
    using MqttCallback = std::function<void(const String& topic, const String& payload)>;

    LTEModule(int rxPin, int txPin, int baud = 115200);

    void begin();
    void loop(); // ⭐ 在这里解析 MQTT 下行
    bool isNetworkReady(); 

    void setCallback(MqttCallback cb); // ⭐ 用户回调注册
        bool mqttConnect(const char* host, int port, const char* clientId,
                     const char* user = nullptr, const char* pass = nullptr,
                     const char* subTopic = nullptr);

private:
    HardwareSerial _serial;
    int _rxPin, _txPin;
    int _baud;

    MqttCallback _cb;
    bool _mqttStarted = false;

    bool sendAT(const String& cmd, const String& expect, uint32_t timeout = 2000);
    void handleMQTTResponse(const String& resp); // ⭐ 内部解析函数
};
