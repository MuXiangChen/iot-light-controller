#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

class ScreenLog
{
public:
    static ScreenLog &instance();

    void begin(int sda, int scl, uint8_t addr = 0x3C);

    // 异步队列日志（线程安全）
    void pushLog(const String &msg);
    void pushLogf(const char *fmt, ...);

    void process(); // 在 loop() 调用

    void clear();

private:
    // 写日志（立即显示）
    void log(const String &msg);
    void logf(const char *fmt, ...);
    ScreenLog();
    ScreenLog(const ScreenLog &) = delete;
    ScreenLog &operator=(const ScreenLog &) = delete;

private:
    void _lock();
    void _unlock();
    void _render();

private:
    static const int MAX_LINES = 8;
    String _lines[MAX_LINES];
    int _lineCount = 0;

    Adafruit_SSD1306 *_display = nullptr;
    SemaphoreHandle_t _mutex = nullptr;
    QueueHandle_t _queue = nullptr;

    uint8_t _addr = 0x3C;
};
