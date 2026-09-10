#pragma once

#include <Arduino.h>

struct SensorState
{
    bool enabled = true;
    bool enabledDirty = false;
    
    bool connected = false;

    float temperatureC = NAN;
};

struct AppState
{
    SensorState sensor1;
    SensorState sensor2;
};