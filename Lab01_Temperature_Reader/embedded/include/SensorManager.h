#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "AppState.h"
#include "Config.h"

class SensorManager
{
public:
    explicit SensorManager(AppState& state);

    void begin();
    void read();

private:
    AppState& state_;

    OneWire oneWire1_;
    OneWire oneWire2_;

    DallasTemperature sensor1_;
    DallasTemperature sensor2_;

    void readSensor(
        DallasTemperature& sensor,
        SensorState& state,
        const char* name
    );
};