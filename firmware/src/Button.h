#pragma once
#include <Arduino.h>
#include <functional>

class Button {
public:
    using Callback = std::function<void()>;

    Button(uint8_t pin, Callback cb);
    void begin();
    void handle(); // 去抖 + 回调执行

private:
    static void IRAM_ATTR isrRouter(void* arg);

    uint8_t _pin;
    Callback _callback;
    volatile bool _pressed = false;
    unsigned long _lastInterruptTime = 0;
};
