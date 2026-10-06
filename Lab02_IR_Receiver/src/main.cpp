#include <Arduino.h>

constexpr int IR_PIN  = 32;  // ADC1_CH4
constexpr int LED_PIN = 12;

// Temporary threshold.
// Adjust this after looking at the serial output.
constexpr int DETECTION_THRESHOLD = 300;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);

    // Lamp OFF initially assumed -> LED ON
    digitalWrite(LED_PIN, HIGH);

    // ESP32 ADC configuration
    analogReadResolution(12);

    // GPIO32 is ADC1_CH4
    analogSetPinAttenuation(IR_PIN, ADC_11db);

    Serial.println();
    Serial.println("==============================");
    Serial.println("IR Receiver Test");
    Serial.println("==============================");
    Serial.println("ADC input: GPIO32");
    Serial.println("LED:       GPIO12");
    Serial.println();
}

void loop() {
    constexpr int NUM_SAMPLES = 50;

    uint16_t minValue = 4095;
    uint16_t maxValue = 0;
    uint32_t sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint16_t sample = analogRead(IR_PIN);

        sum += sample;

        if (sample < minValue)
            minValue = sample;

        if (sample > maxValue)
            maxValue = sample;

        delayMicroseconds(100);
    }

    uint16_t average = sum / NUM_SAMPLES;
    uint16_t peakToPeak = maxValue - minValue;

    bool lampOn = peakToPeak > DETECTION_THRESHOLD;

    // Lamp ON  -> LED OFF
    // Lamp OFF -> LED ON
    digitalWrite(LED_PIN, lampOn ? LOW : HIGH);

    Serial.printf(
        "min=%u max=%u avg=%u pp=%u lamp=%s\n",
        minValue,
        maxValue,
        average,
        peakToPeak,
        lampOn ? "ON" : "OFF"
    );

    delay(5);
}