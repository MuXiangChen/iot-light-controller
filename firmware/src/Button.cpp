#include "Button.h"

Button::Button(uint8_t pin, Callback cb)
    : _pin(pin), _callback(cb) {}

void Button::begin() {
    pinMode(_pin, INPUT_PULLUP);

    // 绑定 ISR （按下为下降沿）
    attachInterruptArg(_pin, Button::isrRouter, this, FALLING);
}

void IRAM_ATTR Button::isrRouter(void* arg) {
    Button* btn = static_cast<Button*>(arg);

    bool pinState = digitalRead(btn->_pin);
    unsigned long now = millis();

    // 检测合法按下（下降沿，而且之前是 HIGH）
    if (!btn->_pressed && pinState == LOW) {
        if (now - btn->_lastInterruptTime > 200) {
            btn->_pressed = true;
            btn->_lastInterruptTime = now;
        }
    }
}


void Button::handle() {
    if (_pressed) {
        _pressed = false;
        if (_callback) _callback();
    }
}
