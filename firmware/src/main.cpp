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
#include "LEDStatus.h"

NetworkManager netManager;
DeviceCore deviceCore;
LEDStatus ledStatus(RGB_PIN);
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
    ledStatus.changeColor(LED_GREEN);
  }
  else if (netType == NET_4G)
  {
    mqtt.select4G();
    ledStatus.changeColor(LED_GREEN);
  }
  else
  {
    // 无网络
    ledStatus.changeColor(LED_RED);
  }
}

void onMqttMsg(String topic, JsonDocument doc)
{
  Serial.printf("🔥 MQTT 收到: %s = %s\n", topic.c_str(), doc.as<String>().c_str());

  if (doc.containsKey("power"))
  {
    deviceCore.powerOn = doc["power"];
    deviceCore.saveConfig();
  }

  if (doc.containsKey("mode"))
  {
    String modeStr = doc["mode"].as<String>();
    deviceCore.autoDim = (modeStr == "auto");
    deviceCore.saveConfig();
  }

  if (doc.containsKey("brightness"))
  {
    float brightness = doc["brightness"].as<float>();
    deviceCore.lightValue = constrain(int(brightness * 25.5), 0, 255);
    deviceCore.saveConfig();
  }

  if (doc.containsKey("markMax"))
  {
    bool state = doc["markMax"];
    if (state)
    {
      deviceCore.sensor_max = analogRead(LDR_PIN);
      deviceCore.saveConfig();
    }
  }

  if (doc.containsKey("markMin"))
  {
    bool state = doc["markMin"];
    if (state)
    {
      deviceCore.sensor_min = analogRead(LDR_PIN);
      deviceCore.saveConfig();
    }
  }
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
  ledStatus.begin();
  ledStatus.changeColor(LED_BLUE);

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

  //   int value = analogRead(KNOB_PIN);  // 读取旋钮电压
  //   deviceCore.lightValue = map(value, 0, 4095, 0, 255); // 映射到 0-255 范围

  // Serial.print("Light value: ");
  // Serial.println(deviceCore.lightValue);
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
