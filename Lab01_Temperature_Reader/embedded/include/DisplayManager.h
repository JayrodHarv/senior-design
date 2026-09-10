#pragma once

#include <Arduino.h>
#include <LiquidCrystal.h>

#include "AppState.h"

class DisplayManager
{
public:
    DisplayManager();

    void begin();

    void update(
        const AppState& state,
        bool wifiConnected
    );

private:
    LiquidCrystal lcd_;

    unsigned long lastUpdate_ = 0;

    void displaySensor(
        uint8_t row,
        uint8_t number,
        const SensorState& sensor
    );

    void printLine(
        uint8_t row,
        const char* text
    );
};