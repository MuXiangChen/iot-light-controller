#include "ScreenUI.h"

ScreenUI& ScreenUI::instance()
{
    static ScreenUI inst;
    return inst;
}

void ScreenUI::begin(int sda, int scl, uint8_t addr)
{
    _addr = addr;
    Wire.begin(sda, scl);

    _display = new Adafruit_SSD1306(128, 64, &Wire, -1);
    _display->begin(SSD1306_SWITCHCAPVCC, _addr);
    _display->clearDisplay();
    _display->display();
}

void ScreenUI::setNetworkStatus(NetworkType net)
{
    _net = net;
}

void ScreenUI::setProvisioning(bool active)
{
    _provisioning = active;
}

void ScreenUI::setMqttStatus(bool connected)
{
    _mqttConnected = connected;
}

void ScreenUI::setDeviceId(const String& id)
{
    _deviceId = id;
}

void ScreenUI::addMessage(const String& msg)
{
    if (_msgCount < MAX_MSG) {
        _messages[_msgCount++] = msg;
    } else {
        for (int i = 1; i < MAX_MSG; i++)
            _messages[i - 1] = _messages[i];
        _messages[MAX_MSG - 1] = msg;
    }
}

void ScreenUI::drawStatusBar()
{
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(0, 0);

    // 网络状态
    String netStr;
    switch (_net)
    {
        case NET_WIFI: netStr = "WiFi"; break;
        case NET_4G:   netStr = "4G";   break;
        default:       netStr = "NoNet"; break;
    }

    // 配网状态
    String provStr = _provisioning ? "Prov" : "";

    // MQTT状态
    String mqttStr = _mqttConnected ? "MQTT:OK" : "MQTT:X";

    // 最终一行（自动控制宽度）
    _display->printf("%s | %s | %s | %s",
                     netStr.c_str(),
                     mqttStr.c_str(),
                     provStr.c_str(),
                     _deviceId.c_str());
}

void ScreenUI::drawMessages()
{
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);

    int y = 12; // 留出顶部状态栏高度
    for (int i = 0; i < _msgCount; i++)
    {
        _display->setCursor(0, y);
        _display->println(_messages[i]);
        y += 10;
    }
}

void ScreenUI::render()
{
    if (!_display) return;

    _display->clearDisplay();

    drawStatusBar();
    drawMessages();

    _display->display();
}
