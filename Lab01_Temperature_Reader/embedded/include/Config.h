#pragma once

#include <Arduino.h>

namespace Config
{

    // -------------------------------------------------
    // Wi-Fi setup access point
    // -------------------------------------------------

    constexpr const char* AP_NAME =
        "TemperatureMonitor-Setup";

    constexpr const char* AP_PASSWORD =
        "TempSetup123";

    // -------------------------------------------------
    // Temperature sensors
    // -------------------------------------------------

    constexpr uint8_t SENSOR_1_PIN = 4;
    constexpr uint8_t SENSOR_2_PIN = 5;

    // -------------------------------------------------
    // Buttons
    // -------------------------------------------------

    constexpr uint8_t BUTTON_1_PIN = 12;
    constexpr uint8_t BUTTON_2_PIN = 13;

    // -------------------------------------------------
    // LCD
    //
    // LCD wiring:
    // RS -> GPIO 14
    // EN -> GPIO 27
    // D4 -> GPIO 18
    // D5 -> GPIO 19
    // D6 -> GPIO 21
    // D7 -> GPIO 22
    //
    // LCD RW should be connected to GND.
    // -------------------------------------------------

    constexpr uint8_t LCD_RS = 14;
    constexpr uint8_t LCD_EN = 27;
    constexpr uint8_t LCD_D4 = 26;
    constexpr uint8_t LCD_D5 = 25;
    constexpr uint8_t LCD_D6 = 33;
    constexpr uint8_t LCD_D7 = 32;

    constexpr uint8_t LCD_COLUMNS = 16;
    constexpr uint8_t LCD_ROWS = 2;

    // -------------------------------------------------
    // Timing
    // -------------------------------------------------

    constexpr unsigned long SENSOR_INTERVAL_MS = 1000;
    constexpr unsigned long STATE_CHECK_INTERVAL_MS = 5000;
    constexpr unsigned long DISPLAY_INTERVAL_MS = 50;
    constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
    constexpr unsigned long BUTTON_DEBOUNCE_MS = 50;

    constexpr unsigned long NETWORK_CONFIG_HOLD_MS = 3000;
}
