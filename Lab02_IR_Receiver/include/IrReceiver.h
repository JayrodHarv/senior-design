
#pragma once

#include <Arduino.h>
#include "Config.h"

class IRReceiver {
public:
    IRReceiver();

    void begin();
    void update();

    bool isDetected() const;
    double getSignalStrength() const;

private:
    double inputHistory_[Config::FILTER_SIZE] = {};
    double outputHistory_[Config::FILTER_SIZE] = {};
    double magnitudeHistory_[Config::HISTORY_SIZE] = {};

    uint32_t lastSampleUs_ = 0;
    uint32_t lastCheckUs_ = 0;

    double signalStrength_ = 0.0;
    bool detected_ = false;

    void sample();
    void checkDetection();
};
