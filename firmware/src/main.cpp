#include <Arduino.h>
#include <DeviceCore.h>
#include "Button.h"
#include "config.h"
#include "NetManager.h"
#include "MQTTManager.h"
#include "ScreenLog.h"
#include "ScreenUI.h"
#include <WiFi.h>
// #include <WiFiClientSecure.h>

NetworkManager netManager;
DeviceCore deviceCore;

Button resetButton(RESET_PIN, []()
                   {
                     Serial.println("重置按钮按下！");
                     netManager.clearCredentials();
                     deviceCore.clearConfig();
                     // nvs.clearConfig();
                     esp_restart(); });

WiFiClient wifiClient;
// WiFiClientSecure wifiClientSecure;

#if BUILD_WITH_4G

#define TINY_GSM_MODEM_ML307
#include <TinyGsmClientML307.h>

HardwareSerial ModemSerial(1);
TinyGsmML307 modem(ModemSerial);
TinyGsmML307::GsmClientML307 lteClient(modem);

MQTTManager mqtt(&wifiClient, &lteClient);
#else

MQTTManager mqtt(&wifiClient);

#endif

void onNetworkChange(NetworkType netType)
{
  ScreenUI::instance().setNetworkStatus(netType);

  if (netType == NET_WIFI)
  {
    mqtt.selectWiFi();
  }
  else if (netType == NET_4G)
  {
    mqtt.select4G();
  }
  else
  {
    // 无网络
  }
}

void onMqttMsg(String topic, JsonDocument doc)
{
  Serial.printf("🔥 MQTT 收到: %s = %s\n", topic.c_str(), doc.as<String>().c_str());
}

void setup()
{
  // ScreenLog::instance().begin(OLED_SDA, OLED_SCL, 0x3C);

  Serial.begin(115200);

  String deviceID = deviceCore.deviceID;
  ScreenUI::instance().begin(OLED_SDA, OLED_SCL);
  ScreenUI::instance().setDeviceId(deviceID);

  deviceCore.autoDimSetup(LDR_PIN, PWM_PIN);
  resetButton.begin();

  netManager.deviceID = deviceID;
  netManager.beginFromNVS();
  netManager.setCallback(onNetworkChange);
  netManager.setupBLEProvisioning(deviceID);
  netManager.startAdvertising();

  // void scanForProvisioning();  // 自动配网

#if BUILD_WITH_4G
  ModemSerial.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
  delay(300);
  // 读取 Modem 信息
  String modemInfo = modem.getModemInfo();
  Serial.println("Modem Info: " + modemInfo);
  netManager.set4Gstatus(modem.isNetworkConnected());

  // wifiClientSecure.setCACert(ca_cert);
#else
  // wifiClientSecure.setCACert(ca_cert);
  Serial.println("Build without 4G support.");
#endif

  /********************************************
   *  mqtt
   ********************************************/
  mqtt.deviceID = deviceID;
  mqtt.init();
  mqtt.setupCallback(onMqttMsg);
}

void loop()
{
#if BUILD_WITH_4G
  static unsigned long last = 0;
  if (millis() - last > 3000)
  {
    last = millis();
    netManager.set4Gstatus(modem.isNetworkConnected());
  }
#endif

  netManager.loop();
  mqtt.loop();

  static unsigned long mqttlast = 0;

  if (millis() - mqttlast > 3000)
  {
    mqttlast = millis();
    ScreenUI::instance().setMqttStatus(mqtt.connected());
  }

  resetButton.handle();
  deviceCore.autoDimLogic();

  ScreenUI::instance().render();
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
