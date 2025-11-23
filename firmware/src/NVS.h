#include <Preferences.h>

class NVS {
public:
    NVS();
    
    Preferences prefs;

    int lightValue = 128;
    bool autoDim = false;
    bool powerOn = true;
    int sensor_min = 1300;
    int sensor_max = 1800;

    void saveConfig();
    void clearConfig();
};