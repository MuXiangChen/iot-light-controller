#include "NVS.h"

NVS::NVS()
{
    // Constructor implementation (if needed)
    prefs.begin("config", false);
    lightValue = prefs.getInt("brightness", 128);
    autoDim = prefs.getBool("autoLight", true);
    powerOn = prefs.getBool("powerOn", true);
    sensor_min = prefs.getInt("sensor_min", 1300);
    sensor_max = prefs.getInt("sensor_max", 1800);
    prefs.end();
}

void NVS::saveConfig()
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

void NVS::clearConfig()
{
    // Code to clear the configuration from non-volatile storage
    prefs.begin("config", false);
    prefs.clear();
    prefs.end();
}