#include <Preferences.h>

class DeviceCore
{
public:
    DeviceCore();
    Preferences prefs;

    String deviceID;

    int lightValue = 128;
    bool autoDim = false;
    bool powerOn = true;
    int sensor_min = 1300;
    int sensor_max = 1800;

    void saveConfig();
    void clearConfig();

    void autoDimSetup(int sensorPin, int pwmPin); // Placeholder for auto-dimming setup
    void autoDimLogic();                          // Placeholder for auto-dimming logic

private:
    int _ldrPin;
    int _pwmPin;
    float smoothLight = 0.0f; // 平滑光敏值
    int lastBrightness = 0;   // 上次输出的亮度
    unsigned long lastLDRCheck = millis();
};