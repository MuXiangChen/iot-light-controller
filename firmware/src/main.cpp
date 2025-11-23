#include <Arduino.h>
#include <NVS.h>
#include "Button.h"
#include "config.h"
#include "NetManager.h"
#include "LTEModule.h"
#include "MQTTManager.h"

NetworkManager netManager;
// NVS nvs;

Button resetButton(RESET_PIN, []()
                   {
                     Serial.println("重置按钮按下！");
                     netManager.clearCredentials();
                     // nvs.clearConfig();
                     esp_restart();
                   });

// LTEModule modem(16, 17);
// MQTTManager mqtt("broker.hivemq.com", 1883);

void onNetworkChange(NetworkType netType)
{
  Serial.printf("🛜 网络切换为: %d，开始处理 MQTT...\n", netType);

  // mqtt.reconnect(netType);

  if (netType == NET_WIFI)
  {
    // mqtt.connectWiFi(); // 你自己的 MQTT 连接逻辑
  }
  else if (netType == NET_4G)
  {
    // mqtt.connect4G();   // 调用 FourGModem->mqttConnect()
  }
  else
  {
    Serial.println("⚠️ 无网络，断开 MQTT 连接");
    // mqtt.disconnect(); // 无网络
  }
}

// void onMqttMsg(String topic, String payload)
// {
//   Serial.printf("🔥 MQTT 收到: %s = %s\n", topic.c_str(), payload.c_str());
// }

String getDeviceID() {
    uint64_t chipid = ESP.getEfuseMac();  // 获取 MAC（高 2 字节固定厂家 ID）
    
    char idStr[18]; // MAC 转文本: 6 字节 => 12 HEX + 5 分隔符 + 结束符
    sprintf(idStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            (uint8_t)(chipid >> 40),
            (uint8_t)(chipid >> 32),
            (uint8_t)(chipid >> 24),
            (uint8_t)(chipid >> 16),
            (uint8_t)(chipid >> 8),
            (uint8_t)(chipid));

    return String(idStr);
}


void setup()
{
  Serial.begin(115200);

String deviceID = getDeviceID();
Serial.println("📌 Device ID: " + deviceID);


  netManager.beginFromNVS();
  netManager.startBLEProvisioning(deviceID);
  netManager.setCallback(onNetworkChange);
  // netManager.set4GChecker([&]() -> bool
  //                         {
  //                           return modem.isNetworkReady(); // 4G 网络检测
  //                         });

  // nvs.saveConfig();
  resetButton.begin();
  // modem.begin();
  // mqtt.begin(onMqttMsg);
}

void loop()
{
  netManager.loop();
  resetButton.handle();
  // mqtt.loop();
}

/*
#include <Arduino.h>

void setup(){
  Serial.begin(115200);
  Serial.println("BOOT OK!");
}

void loop(){
  delay(1000);
  Serial.println("RUNNING");
}
*/
