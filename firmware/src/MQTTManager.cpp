#include "MQTTManager.h"

// ----------------------------------------------------------------------
MQTTManager::MQTTManager(Client *wifiClient, Client *lteClient)
    : _wifi(wifiClient), _lte(lteClient)
{
    _client = _wifi; // 默认 WiFi
    _mqtt.setClient(*_client);
    // _mqtt.setCallback(MQTTManager::mqttCallback);
    // ⬇ 关键：用 lambda 捕获 this，转发到成员函数
    _mqtt.setCallback(
        [this](char *topic, byte *payload, unsigned int length)
        {
            this->handleMqttMessage(topic, payload, length);
        });
}

// ----------------------------------------------------------------------
void MQTTManager::init()
{
    _host = "broker.emqx.io";
    _port = 1883;
    _mqtt.setServer(_host, _port);

      
    snprintf(topicControl, sizeof(topicControl), "iotlight/%s", deviceID.c_str());

    /*  test broker
     */

    /*
    // 如果需要 TLS 支持，可以使用下面的代码
    _host = "x3304b00.ala.cn-hangzhou.emqxsl.cn";
    _port = 8883;
    _mqtt.setServer(_host, _port);
    */
}

// ----------------------------------------------------------------------
void MQTTManager::setupCallback(MsgCallback cb)
{
    _userCb = cb;
}

// ----------------------------------------------------------------------
void MQTTManager::selectWiFi()
{
    if (_wifi)
    {
        _client = _wifi;
        _mqtt.setClient(*_client);
        _currentNet = NET_WIFI;
    }
}

// ----------------------------------------------------------------------
void MQTTManager::select4G()
{
    if (_lte)
    {
        _client = _lte;
        _mqtt.setClient(*_client);
        _currentNet = NET_4G;
    }
}

// ----------------------------------------------------------------------
void MQTTManager::connectIfNeeded()
{
    if (_mqtt.connected())
        return;

    uint32_t now = millis();
    if (now - _lastRetry < 3000)
        return;
    _lastRetry = now;

    String clientId = "ESP32Client-" + deviceID;

    _mqtt.connect(clientId.c_str(), "emqx", "public");

    _mqtt.subscribe(topicControl);
          
    _mqtt.publish(topicBootReport, deviceID.c_str());
}

// ----------------------------------------------------------------------
void MQTTManager::loop()
{
    if (!_mqtt.connected())
    {
        connectIfNeeded();
    }
    _mqtt.loop();
}

// ----------------------------------------------------------------------
/**********************************************************************
 *  pubsubclient mqtt 回调
 **********************************************************************/

void MQTTManager::handleMqttMessage(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("📥 Message arrived [%s] ", topic);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err)
    {
        Serial.printf("❌ JSON error: %s\n", err.c_str());
        return;
    }

    if (_userCb)
    {
        _userCb(String(topic), doc);
    }
}

// ----------------------------------------------------------------------
void MQTTManager::sendLog(String msg)
{
    if (_mqtt.connected())
    {
        _mqtt.publish("device/log", msg.c_str());
    }
}

void MQTTManager::sendDeviceInfo()
{
    if (_mqtt.connected())
    {
        _mqtt.publish("device/info", "Hello");
    }
}
