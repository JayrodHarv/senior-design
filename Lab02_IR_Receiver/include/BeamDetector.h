#pragma once

#include <Arduino.h>

// Turns the filter's raw beam/no-beam decision into a stable state.
// A change only counts after it has held for the configured confirm time.
class BeamDetector
{
public:
    // Returns true when the stable state changes.
    bool update(bool beamPresent, unsigned long now);

    bool isBroken() const { return broken_; }

    // False until the beam has been seen intact once. Breaks are only
    // reported to the cloud after this, so booting with the transmitter
    // off or misaligned does not send an alert.
    bool isArmed() const { return armed_; }

private:
    bool broken_ = true; // no beam seen yet at boot (LED on)
    bool armed_ = false;
    bool pending_ = false;
    unsigned long pendingSince_ = 0;
};
