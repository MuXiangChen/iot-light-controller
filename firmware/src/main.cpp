#include <Arduino.h>
#include <DeviceCore.h>
#include "Button.h"
#include "config.h"
#include "NetManager.h"
#include "LTEModule.h"
#include "MQTTManager.h"

NetworkManager netManager;
DeviceCore deviceCore;

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

#define UART1_RX 20
#define UART1_TX 21

void setup()
{
  Serial.begin(115200);

      //logging
    Serial.println("DeviceCore initialized with config:");
    Serial.println("  lightValue: " + String(deviceCore.lightValue));
    Serial.println("  autoDim: " + String(deviceCore.autoDim));
    Serial.println("  powerOn: " + String(deviceCore.powerOn));
    Serial.println("  sensor_min: " + String(deviceCore.sensor_min));
    Serial.println("  sensor_max: " + String(deviceCore.sensor_max));  


// String deviceID =   deviceCore.getDeviceID();
Serial.println("📌 Device ID: " + deviceCore.deviceID);
  
deviceCore.autoDimSetup(LDR_PIN, PWM_PIN);

  netManager.beginFromNVS();
  netManager.setupBLEProvisioning(deviceCore.deviceID);
  netManager.setCallback(onNetworkChange);
  // netManager.startAsyncScan();
  // netManager.set4GChecker([&]() -> bool
  //                         {
  //                           return modem.isNetworkReady(); // 4G 网络检测
  //                         });

      // void scanForProvisioning();  // 自动配网


  // nvs.saveConfig();
  resetButton.begin();
  // modem.begin();
  // mqtt.begin(onMqttMsg);
}

void loop()
{
  netManager.loop();
  resetButton.handle();
  deviceCore.autoDimLogic();
  // mqtt.loop();

  // while (Serial1.available()) 
  // {
  //   /* code */
  //   char c = Serial1.read();
  //   Serial.write(c);
  // }
  
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
