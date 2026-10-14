#include <Arduino.h>

const int IR_LED_PIN = 32;
const int FREQUENCY = 440;

void setup() {
    Serial.begin(115200);

    pinMode(IR_LED_PIN, OUTPUT);

    // Generate a 440 Hz square wave (50% duty cycle).
    // 1136 us HIGH + 1137 us LOW ≈ 440 Hz.
    Serial.println("Starting 440 Hz IR transmitter...");
}

void loop() {
    digitalWrite(IR_LED_PIN, HIGH);
    delayMicroseconds(1136);

    digitalWrite(IR_LED_PIN, LOW);
    delayMicroseconds(1137);
}