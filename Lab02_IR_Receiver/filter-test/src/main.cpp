#include <Arduino.h>
#include <math.h>

// Pin configuration
constexpr int IR_PIN = 32;
constexpr int LED_PIN = 12;

// Filter configuration
constexpr int N = 7;
constexpr int M = 10;
constexpr uint32_t SAMPLE_INTERVAL_US = 333;
constexpr uint32_t CHECK_INTERVAL_US = 1000000;

constexpr double THRESHOLD = 0.5;

// Filter coefficients
double num[N] = {
    0.000941313746993275,
    0,
    -0.00282394124097983,
    0,
    0.00282394124097983,
    0,
    -0.000941313746993275
};

double den[N] = {
    1,
    -3.39337775868904,
    6.42931418056668,
    -7.32802209032523,
    5.5902473448203,
    -2.5645446166821,
    0.657274702102658
};

// Sample buffers
double x[N] = {};
double y[N] = {};
double s[M] = {};

uint32_t lastSample = 0;
uint32_t lastCheck = 0;

void setup() {
    Serial.begin(115200);

    pinMode(IR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    // ESP32 ADC configuration
    analogReadResolution(12);
    analogSetPinAttenuation(IR_PIN, ADC_11db);

    lastSample = micros();
    lastCheck = micros();
}

void loop() {
    uint32_t now = micros();

    // Maintain 3000 Hz sampling rate
    if ((uint32_t)(now - lastSample) < SAMPLE_INTERVAL_US) {
        return;
    }

    lastSample += SAMPLE_INTERVAL_US;

    // Shift previous samples
    for (int i = N - 1; i > 0; i--) {
        x[i] = x[i - 1];
        y[i] = y[i - 1];
    }

    // Shift hysteresis samples
    for (int i = M - 1; i > 0; i--) {
        s[i] = s[i - 1];
    }

    // Read ESP32's 12-bit ADC (0-4095)
    int val = analogRead(IR_PIN);

    // Scale ADC to approximately -1.65V to +1.65V
    x[0] = val * (3.3 / 4095.0) - 1.65;

    // Calculate filter output
    double yn = num[0] * x[0];

    for (int i = 1; i < N; i++) {
        yn += num[i] * x[i] - den[i] * y[i];
    }

    y[0] = yn;

    // Absolute filter output
    s[0] = fabs(2.0 * yn);

    // Find maximum of recent samples
    double maxs = 0.0;

    for (int i = 0; i < M; i++) {
        if (s[i] > maxs) {
            maxs = s[i];
        }
    }

    // Check threshold once per second
    if ((uint32_t)(now - lastCheck) >= CHECK_INTERVAL_US) {
        lastCheck = now;

        Serial.println(maxs, 6);

        if (maxs < THRESHOLD) {
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
    }
}