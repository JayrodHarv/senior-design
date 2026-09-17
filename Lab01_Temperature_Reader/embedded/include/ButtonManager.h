#pragma once

#include <Arduino.h>
#include <esp_timer.h>
#include "AppState.h"

class ButtonManager
{
public:
    explicit ButtonManager(AppState& state);

    void begin();
    uint8_t update(); // Bit mask of sensors toggled this update.

    bool consumeNetworkConfigRequest();

private:
    AppState& state_;

    struct DebouncedButton
    {
        bool rawDown = false;
        bool stableDown = false;
        unsigned long changedAt = 0;

        bool sample(bool down, unsigned long now);
    };

    DebouncedButton button1_;
    DebouncedButton button2_;
    esp_timer_handle_t sampleTimer_ = nullptr;

    // The timer owns debounce state; loop() owns AppState.
    portMUX_TYPE eventMux_ = portMUX_INITIALIZER_UNLOCKED;
    uint8_t pendingToggles_ = 0;
    bool networkConfigRequested_ = false;

    unsigned long bothButtonsPressedSince_ = 0;
    bool bothButtonsDown_ = false;
    bool networkConfigTriggered_ = false;

    static void sampleButtons(void* context);
    void sampleButtons();

    void toggleSensor(
        SensorState& sensor,
        const char* name
    );
};