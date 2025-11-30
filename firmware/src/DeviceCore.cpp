#include "DeviceCore.h"

DeviceCore::DeviceCore()
{
    // Constructor implementation (if needed)
    prefs.begin("config", false);
    lightValue = prefs.getInt("brightness", 128);
    autoDim = prefs.getBool("autoLight", true);
    powerOn = prefs.getBool("powerOn", true);
    sensor_min = prefs.getInt("sensor_min", 1300);
    sensor_max = prefs.getInt("sensor_max", 1800);
    prefs.end();

    // deviceID = getDeviceID();
    uint64_t chipid = ESP.getEfuseMac(); // 获取 MAC（高 2 字节固定厂家 ID）

    char idStr[18]; // MAC 转文本: 6 字节 => 12 HEX + 5 分隔符 + 结束符
    sprintf(idStr, "%02X%02X%02X%02X%02X%02X",
            (uint8_t)(chipid >> 40),
            (uint8_t)(chipid >> 32),
            (uint8_t)(chipid >> 24),
            (uint8_t)(chipid >> 16),
            (uint8_t)(chipid >> 8),
            (uint8_t)(chipid));
    // sprintf(idStr, "%02X:%02X:%02X:%02X:%02X:%02X",
    //         (uint8_t)(chipid >> 40),
    //         (uint8_t)(chipid >> 32),
    //         (uint8_t)(chipid >> 24),
    //         (uint8_t)(chipid >> 16),
    //         (uint8_t)(chipid >> 8),
    //         (uint8_t)(chipid));
    deviceID = String(idStr);
}

void DeviceCore::saveConfig()
{
    // Code to save the configuration to non-volatile storage
    prefs.begin("config", false);
    prefs.putInt("brightness", lightValue);
    prefs.putBool("autoLight", autoDim);
    prefs.putBool("powerOn", powerOn);
    prefs.putInt("sensor_min", sensor_min);
    prefs.putInt("sensor_max", sensor_max);
    prefs.end();
}

void DeviceCore::clearConfig()
{
    // Code to clear the configuration from non-volatile storage
    prefs.begin("config", false);
    prefs.clear();
    prefs.end();
}

void DeviceCore::autoDimSetup(int sensorPin, int pwmPin)
{
    _ldrPin = sensorPin;
    _pwmPin = pwmPin;
    smoothLight = 0.0f;
    lastBrightness = 0;

    // PWM 初始化
    ledcSetup(0, 5000, 8);
    ledcAttachPin(_pwmPin, 0);
    ledcWrite(0, 128);

    pinMode(_ldrPin, INPUT);
}

void DeviceCore::autoDimLogic()
{
            // ledcWrite(0, lightValue);
            // return;


    // 自动亮度（每0.3秒执行一次）
    unsigned long now = millis();
    if (now - lastLDRCheck < 300)
    {
        return;
    }

    lastLDRCheck = now;

    if (!powerOn)
    {
        ledcWrite(0, 0);
        return;
    }

    if (autoDim)
    {
        // 原始光敏值
        int rawLight = analogRead(_ldrPin);

        // 低通滤波（指数平滑）—— alpha 越小越稳
        smoothLight = 0.9f * smoothLight + 0.1f * rawLight;

        // 先把钳位上下限理顺：无论 min/max 谁大谁小，都正确钳位
        int low = min(sensor_min, sensor_max);
        int high = max(sensor_min, sensor_max);
        int filteredLight = constrain((int)smoothLight, low, high);

        // 比例映射（自动判断方向）
        // 目标：最暗→ratio=0，最亮→ratio=1
        float fMin = (float)sensor_min; // 你记录的“暗光读数”
        float fMax = (float)sensor_max; // 你记录的“亮光读数”
        float fVal = (float)filteredLight;

        float ratio = 0.0f;
        if (fabsf(fMax - fMin) < 1.0f)
        {
            ratio = 0.5f; // 防止除零（未校准或两次读数相同）
        }
        else if (fMax > fMin)
        {
            // 正向（越亮数值越大）
            ratio = (fVal - fMin) / (fMax - fMin);
        }
        else
        {
            // 反向（越亮数值越小）
            ratio = (fMin - fVal) / (fMin - fMax);
        }
        ratio = constrain(ratio, 0.0f, 1.0f);
        ratio = 1 - ratio;

        // 计算亮度 & 死区抑制，避免抖动闪烁
        int targetBrightness = (int)(ratio * lightValue);
        static int lastBrightness = 0;
        if (abs(targetBrightness - lastBrightness) > 5)
        {
            ledcWrite(0, targetBrightness);
            lastBrightness = targetBrightness;
        }

        // 调试输出
        // Serial.printf("🌞 LDR(raw=%d, filt=%.1f, min=%d, max=%d) → Ratio=%.2f → Brightness=%d\n",
        //               rawLight, smoothLight, sensor_min, sensor_max, ratio, lastBrightness);
    }
    else
    {
        ledcWrite(0, lightValue);
    }
    // Placeholder for auto-dimming logic
    // This function can be expanded to include actual sensor reading and dimming logic
}