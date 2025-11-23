#include "LTEModule.h"

LTEModule::LTEModule(int rxPin, int txPin, int baud)
    : _serial(2), _rxPin(rxPin), _txPin(txPin), _baud(baud) {}

void LTEModule::setCallback(MqttCallback cb) {
    _cb = cb;
}

void LTEModule::loop() {
    while (_serial.available()) {
        String line = _serial.readStringUntil('\n');
        line.trim();
        if (line.startsWith("+CMQTTRX")) {
            handleMQTTResponse(line);
        }
    }
}

bool LTEModule::mqttConnect(const char* host, int port, const char* clientId,
                            const char* user, const char* pass,
                            const char* subTopic)
{
    Serial.println("📡 启动 4G MQTT...");

    if (!_mqttStarted) {
        if (!sendAT("AT+CMQTTSTART", "OK", 6000)) {
            Serial.println("❌ CMQTTSTART Fail");
            return false;
        }
        _mqttStarted = true;
    }

    // session 0, clean session=1
    String accq = String("AT+CMQTTACCQ=0,\"") + clientId + "\",1";
    if (!sendAT(accq, "OK", 3000)) return false;

    String conn = String("AT+CMQTTCONNECT=0,\"tcp://") + host + ":" + port + "\"";
    if (user && pass) {
        conn += String(",60,1,\"") + user + "\",\"" + pass + "\"";
    } else {
        conn += ",60,1";
    }

    if (!sendAT(conn, "OK", 8000)) return false;

    Serial.println("✔️ MQTT 连接成功");

    if (subTopic) {
        Serial.printf("📩 订阅 Topic: %s\n", subTopic);
        String subCmd = String("AT+CMQTTSUB=0,") + strlen(subTopic) + ",1,\"" + subTopic + "\"";
        if (!sendAT(subCmd, "OK")) {
            Serial.println("⚠️ 订阅失败");
        }
    }

    return true;
}


void LTEModule::handleMQTTResponse(const String& resp) {
    // 格式示例:
    // +CMQTTRX: 0,3,topic_len,payload_len
    // 然后下一行是 topic
    // 再下一行是 payload

    if (!_cb) return;

    // 读取 topic 行
    while(!_serial.available());
    String topic = _serial.readStringUntil('\n');
    topic.trim();

    // 读取 payload 行
    while(!_serial.available());
    String payload = _serial.readStringUntil('\n');
    payload.trim();

    Serial.printf("📥 4G MQTT RX: %s -> %s\n", topic.c_str(), payload.c_str());

    if (_cb) _cb(topic, payload);
}

void LTEModule::begin() {
    _serial.begin(_baud, SERIAL_8N1, _rxPin, _txPin);
    delay(300);
    sendAT("AT", "OK");
    sendAT("ATE0", "OK");
    sendAT("AT+CFUN=1", "OK");
    sendAT("AT+CREG=1", "OK");
    sendAT("AT+CGATT=1", "OK");
}

bool LTEModule::isNetworkReady() {
    _serial.flush();
    _serial.println("AT+CREG?");
    uint32_t start = millis();
    while (millis() - start < 1000) {
        if (_serial.find(",1") || _serial.find(",5")) return true;
    }
    return false;
}

bool LTEModule::sendAT(const String& cmd, const String& expect, uint32_t timeout) {
    _serial.println(cmd);
    uint32_t start = millis();
    while (millis() - start < timeout) {
        if (_serial.find(const_cast<char*>(expect.c_str()))) return true;
    }
    Serial.printf("⚠️ AT timeout: %s\n", cmd.c_str());
    return false;
}
