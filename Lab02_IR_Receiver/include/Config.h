#pragma once

#include <Arduino.h>

namespace Config
{
// ---------------- Wi-Fi setup portal (used by NetworkManager) -------------
constexpr const char* AP_NAME = "BeamBreak-Setup";
constexpr const char* AP_PASSWORD = "beam1234"; // WiFiManager needs >= 8 chars

// ---------------- Beam-break debounce -------------------------------------
// Applied to the digital filter's output so a single noisy filter block
// cannot trigger (or clear) an alert.
constexpr uint32_t BEAM_BREAK_CONFIRM_MS = 100;   // beam missing this long = broken
constexpr uint32_t BEAM_RESTORE_CONFIRM_MS = 300; // beam back this long = restored

// ---------------- Cloud ---------------------------------------------------
constexpr int BEAM_ROW_ID = 1;              // row in public.beam_state
constexpr uint16_t HTTP_TIMEOUT_MS = 3000;

// ---------------- IR receiver ---------------------------------------------
constexpr uint8_t IR_PIN = 32;
constexpr uint8_t LED_PIN = 12;

constexpr int FILTER_ORDER = 6;
constexpr int FILTER_SIZE = FILTER_ORDER + 1;
constexpr int HISTORY_SIZE = 10;

// 333 us is the interval used when the filter was designed (~3000 Hz).
constexpr uint32_t SAMPLE_INTERVAL_US = 333;
constexpr uint32_t CHECK_INTERVAL_US = 1000000;
constexpr double DETECTION_THRESHOLD = 0.5;

constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr int ADC_MAX = 4095;
constexpr double ADC_VOLTAGE = 3.3;
constexpr double ADC_OFFSET = ADC_VOLTAGE / 2.0;
constexpr double FILTER_OUTPUT_SCALE = 2.0;

constexpr double FILTER_NUMERATOR[FILTER_SIZE] = {
    0.000941313746993275,
    0,
    -0.00282394124097983,
    0,
    0.00282394124097983,
    0,
    -0.000941313746993275
};

constexpr double FILTER_DENOMINATOR[FILTER_SIZE] = {
    1,
    -3.39337775868904,
    6.42931418056668,
    -7.32802209032523,
    5.5902473448203,
    -2.5645446166821,
    0.657274702102658
};
}
