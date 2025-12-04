#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

enum LEDColor
{
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_YELLOW,
    LED_CYAN,
    LED_PURPLE,
    LED_CUSTOM,
    LED_OFF
};

class LEDStatus
{
public:
    LEDStatus(uint8_t pin, uint16_t count = 1);

    void begin(uint8_t brightness = 50);

    // 切换预设颜色
    void changeColor(LEDColor color);

    // 自定义颜色（并切到 LED_CUSTOM 模式）
    void setCustomColor(uint8_t r, uint8_t g, uint8_t b);

    // 设置亮度（0~255）
    void setBrightness(uint8_t brightness);

private:
    void applyColor();   // 把当前 _customColor 写到所有像素

private:
    uint8_t _pin;
    uint16_t _count;
    Adafruit_NeoPixel _pixels;
    uint32_t _customColor = 0;   // 当前颜色
};
