#include "MQTTManager.h"

extern LTEModule modem; // 使用你已有的 4G 对象

static MQTTManager *mqttInstance = nullptr;

MQTTManager::MQTTManager() : _mqtt(_wifiClient)
{
    mqttInstance = this; // ⭐ 保存实例，供静态回调转发用
}

void MQTTManager::init()
{
    _host = "broker.hivemq.com";
    _port = 1883;
}

void MQTTManager::setupCallback(MsgCallback cb)
{
    _userCb = cb;
}

void MQTTManager::setLTEModule(LTEModule *modem)
{
    _modem = modem;
}

void MQTTManager::connectViaWiFi()
{
    _currentNet = NET_WIFI;

    _mqtt.setServer(_host, _port);
    _mqtt.setCallback(MQTTManager::mqttCallback); // ⭐绑定静态转发器
}

void MQTTManager::connectVia4G()
{
    _currentNet = NET_4G;

    if (_modem)
    {
        _modem->mqttConnect(_host, _port, "iot-device-client");
    }
}

void MQTTManager::loop()
{
    if (_currentNet == NET_WIFI)
    {
        _mqtt.loop();
    }
    // 4G 消息接收由 LTEModule.loop() 实现
}

void MQTTManager::onMessage(char *topic, byte *payload, unsigned int length)
{
    String t = String(topic);
    String p;

    for (uint i = 0; i < length; i++)
    {
        p += (char)payload[i];
    }

    Serial.printf("🔥 收到 MQTT: %s = %s\n", t.c_str(), p.c_str());

    if (_userCb)
    {
        _userCb(t, p); // ⭐ 转发给用户代码
    }
}

void MQTTManager::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    if (mqttInstance)
    {
        mqttInstance->onMessage(topic, payload, length);
    }
}

/********************************************
 *  业务
 ********************************************/

void MQTTManager::sendLog(String message)
{
    if (_currentNet == NET_4G)
    {
        /* code */
    }
    else
    {
        if (_mqtt.connected())
        {
            _mqtt.publish("device/log", message.c_str());
        }
    }
}

void MQTTManager::sendDeviceInfo()
{
    if (_currentNet == NET_4G)
    {
        /* code */
    }
    else
    {
        if (_mqtt.connected())
        {
            String info = "{\"device\":\"iot-light-controller\",\"version\":\"1.0.0\"}";
            _mqtt.publish("device/info", info.c_str());
        }
    }
}