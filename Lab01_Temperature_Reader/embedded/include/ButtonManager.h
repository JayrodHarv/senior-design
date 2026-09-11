#pragma once

#include <Arduino.h>
#include "AppState.h"

class ButtonManager
{
public:
    explicit ButtonManager(AppState& state);

    void begin();
    void update();

    bool consumeNetworkConfigRequest();

private:
    AppState& state_;

    static volatile bool button1Pressed_;
    static volatile bool button2Pressed_;

    static void IRAM_ATTR handleButton1Interrupt();
    static void IRAM_ATTR handleButton2Interrupt();

    unsigned long bothButtonsPressedSince_ = 0;

    bool networkConfigRequested_ = false;
    bool networkConfigTriggered_ = false;

    void toggleSensor(
        SensorState& sensor,
        const char* name
    );
};