#include "IrReceiver.h"

#include <math.h>

IRReceiver::IRReceiver() = default;

void IRReceiver::begin()
{
    pinMode(Config::IR_PIN, INPUT);
    pinMode(Config::LED_PIN, OUTPUT);
    digitalWrite(Config::LED_PIN, LOW);

    analogReadResolution(Config::ADC_RESOLUTION_BITS);
    analogSetPinAttenuation(Config::IR_PIN, ADC_11db);

    const uint32_t now = micros();
    lastSampleUs_ = now;
    lastCheckUs_ = now;
}

void IRReceiver::update()
{
    const uint32_t now = micros();

    if (static_cast<uint32_t>(now - lastSampleUs_) >=
        Config::SAMPLE_INTERVAL_US)
    {
        // Advancing by the configured interval avoids accumulating drift.
        lastSampleUs_ += Config::SAMPLE_INTERVAL_US;
        sample();
    }

    if (static_cast<uint32_t>(now - lastCheckUs_) >=
        Config::CHECK_INTERVAL_US)
    {
        lastCheckUs_ = now;
        checkDetection();
    }
}

bool IRReceiver::isDetected() const
{
    return detected_;
}

double IRReceiver::getSignalStrength() const
{
    return signalStrength_;
}

void IRReceiver::sample()
{
    for (int i = Config::FILTER_SIZE - 1; i > 0; --i)
    {
        inputHistory_[i] = inputHistory_[i - 1];
        outputHistory_[i] = outputHistory_[i - 1];
    }

    for (int i = Config::HISTORY_SIZE - 1; i > 0; --i)
    {
        magnitudeHistory_[i] = magnitudeHistory_[i - 1];
    }

    const int adcValue = analogRead(Config::IR_PIN);
    inputHistory_[0] = adcValue *
        (Config::ADC_VOLTAGE / Config::ADC_MAX) - Config::ADC_OFFSET;

    double output = Config::FILTER_NUMERATOR[0] * inputHistory_[0];
    for (int i = 1; i < Config::FILTER_SIZE; ++i)
    {
        output += Config::FILTER_NUMERATOR[i] * inputHistory_[i]
            - Config::FILTER_DENOMINATOR[i] * outputHistory_[i];
    }

    outputHistory_[0] = output;
    magnitudeHistory_[0] = fabs(Config::FILTER_OUTPUT_SCALE * output);

    signalStrength_ = 0.0;
    for (int i = 0; i < Config::HISTORY_SIZE; ++i)
    {
        if (magnitudeHistory_[i] > signalStrength_)
        {
            signalStrength_ = magnitudeHistory_[i];
        }
    }
}

void IRReceiver::checkDetection()
{
    detected_ = signalStrength_ >= Config::DETECTION_THRESHOLD;

    Serial.println(signalStrength_, 6);
    digitalWrite(Config::LED_PIN, detected_ ? LOW : HIGH);
}
