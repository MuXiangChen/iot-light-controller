#include "LEDStatus.h"

LEDStatus::LEDStatus(uint8_t pin, uint16_t count)
    : _pin(pin),
      _count(count),
      _pixels(count, pin, NEO_GRB + NEO_KHZ800)
{
}

void LEDStatus::begin(uint8_t brightness)
{
    _pixels.begin();
    _pixels.clear();
    _pixels.setBrightness(brightness);
    _pixels.show();  // 全部熄灭
}

void LEDStatus::setBrightness(uint8_t brightness)
{
    _pixels.setBrightness(brightness);
    _pixels.show();  // 更新亮度
}

void LEDStatus::setCustomColor(uint8_t r, uint8_t g, uint8_t b)
{
    _customColor = _pixels.Color(r, g, b);
    applyColor();
}

void LEDStatus::changeColor(LEDColor color)
{
    switch (color)
    {
    case LED_RED:
        _customColor = _pixels.Color(255, 0, 0);
        break;
    case LED_GREEN:
        _customColor = _pixels.Color(0, 255, 0);
        break;
    case LED_BLUE:
        _customColor = _pixels.Color(0, 0, 255);
        break;
    case LED_YELLOW:
        _customColor = _pixels.Color(255, 255, 0);
        break;
    case LED_CYAN:
        _customColor = _pixels.Color(0, 255, 255);
        break;
    case LED_PURPLE:
        _customColor = _pixels.Color(255, 0, 255);
        break;
    case LED_OFF:
        _customColor = _pixels.Color(0, 0, 0);
        break;
    case LED_CUSTOM:
    default:
        // 保持 _customColor 不变
        break;
    }

    applyColor();
}

void LEDStatus::applyColor()
{
    for (uint16_t i = 0; i < _count; i++)
    {
        _pixels.setPixelColor(i, _customColor);
    }
    _pixels.show();
}
