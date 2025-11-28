#include "ScreenLog.h"

// ----------- 单例 -----------
ScreenLog &ScreenLog::instance()
{
    static ScreenLog inst;
    return inst;
}

// ----------- 构造函数 -----------
ScreenLog::ScreenLog()
{
}

// ----------- 初始化 OLED -----------
void ScreenLog::begin(int sda, int scl, uint8_t addr)
{
    _addr = addr;

    Wire.begin(sda, scl);

    if (!_display)
        _display = new Adafruit_SSD1306(128, 64, &Wire, -1);

    _display->begin(SSD1306_SWITCHCAPVCC, _addr);
    _display->clearDisplay();
    _display->display();

    // 创建同步设施
    _mutex = xSemaphoreCreateMutex();
    _queue = xQueueCreate(20, sizeof(char[128]));
}

// ----------- 线程安全写日志（立即显示） -----------
void ScreenLog::log(const String &text)
{
    _lock();

    if (_lineCount < MAX_LINES)
    {
        _lines[_lineCount++] = text;
    }
    else
    {
        for (int i = 1; i < MAX_LINES; i++)
            _lines[i - 1] = _lines[i];

        _lines[MAX_LINES - 1] = text;
    }

    _render();

    _unlock();
}

// ----------- printf 风格日志 -----------
void ScreenLog::logf(const char *fmt, ...)
{
    char buf[128];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    log(String(buf));
}

// ----------- 清屏 -----------
void ScreenLog::clear()
{
    _lock();
    _lineCount = 0;
    if (_display)
    {
        _display->clearDisplay();
        _display->display();
    }
    _unlock();
}

// ----------- 渲染 OLED 内容 -----------
void ScreenLog::_render()
{
    if (!_display) return;

    _display->clearDisplay();
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    for (int i = 0; i < MAX_LINES; i++) {

        // ---- 关键：先清除这一行 8px 的区域 ----
        _display->fillRect(0, i * 8, 128, 8, SSD1306_BLACK);

        // 写入文本
        _display->setCursor(0, i * 8);
        if (i < _lineCount)
            _display->print(_lines[i]);   // 注意：用 print，不要 println！
    }

    _display->display();
}


// ----------- 异步日志，放入队列 -----------
void ScreenLog::pushLog(const String &msg)
{
    if (!_queue)
        return;

    char buf[128];
    strncpy(buf, msg.c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;

    xQueueSend(_queue, buf, 0); // 非阻塞
}

// 格式化版本
void ScreenLog::pushLogf(const char *fmt, ...)
{
    if (!_queue)
        return;

    char buf[128];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // 严格保证字符串以 '\0' 结尾
    buf[sizeof(buf) - 1] = 0;

    xQueueSend(_queue, buf, 0);
}

// ----------- 主线程调用：处理队列日志 -----------
void ScreenLog::process()
{
    static unsigned long last = 0;
    if (millis() - last < 30)
        return;
    last = millis();

    if (!_queue)
        return;

    char buf[128];
    while (xQueueReceive(_queue, buf, 0) == pdTRUE)
    {
        log(String(buf));
    }
}

// ----------- 互斥锁保护（线程安全） -----------
void ScreenLog::_lock()
{
    if (_mutex)
        xSemaphoreTake(_mutex, portMAX_DELAY);
}

void ScreenLog::_unlock()
{
    if (_mutex)
        xSemaphoreGive(_mutex);
}
