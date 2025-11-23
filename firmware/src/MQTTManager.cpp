#include "MQTTManager.h"

extern LTEModule modem;  // 使用你已有的 4G 对象

MQTTManager::MQTTManager(const char* host, int port)
    : _host(host), _port(port), _mqtt(_wifiClient) {}

void MQTTManager::begin(MsgCallback cb) {
    _userCb = cb;
    _mqtt.setServer(_host, _port);

    // WiFi MQTT 回调转发
    _mqtt.setCallback([this](char* topic, byte* payload, unsigned int len) {
        onWiFiMessage(topic, payload, len);
    });
}

void MQTTManager::loop() {
    if (_currentNet == NET_WIFI) {
        _mqtt.loop();
    }
    // 4G 消息接收由 LTEModule.loop() 实现
}

void MQTTManager::reconnect(NetworkType netType) {
    if (millis() - _lastRetry < 2000) return;
    _lastRetry = millis();

    if (netType != _currentNet) {
        Serial.printf("🔄 MQTT 切换网络模式: %d -> %d\n", _currentNet, netType);
        _currentNet = netType;
    }

    if (_currentNet == NET_WIFI) {
        connectViaWiFi();
    } else if (_currentNet == NET_4G) {
        connectVia4G();
    }
}

bool MQTTManager::connectViaWiFi() {
    Serial.println("🔌 MQTT over WiFi Connecting...");
    if (_mqtt.connect("ESP32-WIFI-MQTT")) {
        Serial.println("📶 WiFi MQTT Connected!");
        _mqtt.subscribe("device/cmd");
        return true;
    }
    Serial.println("⚠️ WiFi MQTT Connect Fail");
    return false;
}

bool MQTTManager::connectVia4G() {
    Serial.println("📡 MQTT over 4G Connecting...");

        // ⭐ 设置 4G MQTT 回调
    modem.setCallback([this](const String& topic, const String& payload){
        Serial.printf("📥 MQTT 4G: %s -> %s\n", topic.c_str(), payload.c_str());
        if (_userCb) _userCb(topic, payload); // 统一上抛
    });
    
    return modem.mqttConnect(_host, _port, "ESP32-4G-MQTT",
                             nullptr, nullptr,
                             "device/cmd");
}

void MQTTManager::onWiFiMessage(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned i = 0; i < length; i++) msg += (char)payload[i];

    Serial.printf("📥 MQTT WiFi: %s -> %s\n", topic, msg.c_str());
    if (_userCb)
        _userCb(String(topic), msg);
}
