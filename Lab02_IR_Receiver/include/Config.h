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
}
