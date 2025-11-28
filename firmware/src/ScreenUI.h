#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "NetManager.h"

class ScreenUI
{
public:
    static ScreenUI& instance();

    void begin(int sda, int scl, uint8_t addr = 0x3C);

    void setNetworkStatus(NetworkType net);
    void setProvisioning(bool active);
    void setMqttStatus(bool connected);
    void setDeviceId(const String& id);

    void addMessage(const String& msg);

    void render();  // 主循环中调用

private:
    ScreenUI() {}
    void drawStatusBar();
    void drawMessages();

private:
    Adafruit_SSD1306* _display = nullptr;

    NetworkType _net = NET_NONE;
    bool _provisioning = false;
    bool _mqttConnected = false;
    String _deviceId = "----";

    static const int MAX_MSG = 6;
    String _messages[MAX_MSG];
    int _msgCount = 0;

    uint8_t _addr = 0x3C;
};
