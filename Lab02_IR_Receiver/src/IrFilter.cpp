#include "IrFilter.h"

#include <Arduino.h>



namespace
{
constexpr int TEST_BEAM_PIN = 4;
}

namespace IrFilter
{
void begin()
{
    pinMode(TEST_BEAM_PIN, INPUT_PULLUP);
    Serial.println("[Filter] PLACEHOLDER active (button simulates beam)");
}

bool beamPresent()
{
    return digitalRead(TEST_BEAM_PIN) == HIGH;
}
}
