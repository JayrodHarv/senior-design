
#include <Arduino.h>
#include <math.h>

// ============================================================
// PIN CONFIGURATION
// ============================================================

constexpr uint8_t IR_PIN = 32;
constexpr uint8_t LED_PIN = 12;

// ============================================================
// SAMPLING CONFIGURATION
// ============================================================

// Must match the sampling frequency used in MATLAB.
constexpr uint32_t SAMPLE_RATE = 3000;
constexpr uint32_t SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE;

// ============================================================
// FILTER CONFIGURATION
// ============================================================

// These examples are second-order band-pass filters:
// 3 numerator and 3 denominator coefficients.
//
// Change FILTER_SIZE if your MATLAB filters have more
// coefficients. All three must have the same size.

constexpr int FILTER_SIZE = 13;
constexpr int NUM_FILTERS = 3;

struct FilterConfig {
    const char* name;
    float lowFreq;
    float highFreq;

    float num[FILTER_SIZE];
    float den[FILTER_SIZE];
};

// ============================================================
// THREE DIGITAL FILTERS
// ============================================================

// Example Butterworth band-pass filters designed at Fs=3000 Hz.
// Replace these values with your MATLAB coefficients.
//
// MATLAB:
// [b,a] = butter(1, [low high]/(Fs/2), 'bandpass');
//
// IMPORTANT: a[0] should equal 1 after normalization.

const FilterConfig filters[NUM_FILTERS] = {

    // FILTER 1: 425 - 445 Hz
    {
        "Low Frequency",
        425.0f,
        445.0f,

        // Numerator
        {
            0.000100157636002270,    -0.000731536112928243,    0.00282176761116235,    -0.00724448843718841,    0.0136359320186407,    -0.0196425486817135,    0.0221424918696991,    -0.0196425486817135,    0.0136359320186407,    -0.00724448843718841,    0.00282176761116235,    -0.000731536112928243,    0.000100157636002270
        },

        // Denominator
        {
            1,    -7.33107355711123,    28.3532022414220,    -72.8927894431495,    137.214040055806,    -197.412379305491,    221.968331825814,    -196.142190359306,    135.453992694626,    -71.4947939025635,    27.6304866324637,    -7.09823800499474,    0.962010498954245
        }
    },

    // FILTER 2: 490 - 510 Hz
    {
        "Mid Frequency",
        490.0f,
        510.0f,

        // Numerator
        {
            0.000100157636002270,    -0.000596775733423385,    0.00207603463281020,    -0.00491971311909698,    0.00882514996524183,    -0.0123300397358421,    0.0137882583771791,    -0.0123300397358421,    0.00882514996524183,    -0.00491971311909698,    0.00207603463281020,    -0.000596775733423385,    0.000100157636002270
        },

        // Denominator
        {
            1,    -5.98057528740894,    20.8622764691172,    -49.5082780943866,    88.8214999258277,    -123.944888539870,    138.250930775924,    -123.147417446691,    87.6822033556846,    -48.5587806763760,    20.3305073859120,    -5.79063168117315,    0.962010498954259
        }
    },

    // FILTER 3: 510 - 545 Hz
    {
        "High Frequency",
        510.0f,
        545.0f,

        // Numerator
        {
            0.000100157636002270,    -0.000519470717073175,    0.00171654295144297,    -0.00386690581990967,    0.00677201910883344,    -0.00927523678622471,    0.0103491422572843,    -0.00927523678622472,    0.00677201910883344,    -0.00386690581990967,    0.00171654295144297,    -0.000519470717073175,    0.000100157636002270
        },

        // Denominator
        {
            1,    -5.20586471443594,    17.2511642090243,    -38.9171367069919,    68.1663818720029,    -93.2489678367309,    103.782781791850,    -92.6490032883793,    67.2920343541794,    -38.1707679006491,    16.8114437405200,    -5.04052598531443,    0.962010498954257
        }
    }
};

// ============================================================
// ACTIVE FILTER
// ============================================================

// 0 = Filter 1
// 1 = Filter 2
// 2 = Filter 3

int activeFilter = 0;

// Separate history buffers for the currently active filter.
float x[FILTER_SIZE] = {};
float y[FILTER_SIZE] = {};

// ============================================================
// DETECTION CONFIGURATION
// ============================================================

// These are example thresholds for normalized ADC samples.
// Adjust after observing filtered RMS with lamp ON and OFF.

float PRESENT_THRESHOLD = 0.04f;
float ABSENT_THRESHOLD  = 0.02f;

// Signal-strength smoothing time constant: 20 ms.
constexpr float SMOOTHING_TIME = 0.020f;

const float RMS_ALPHA =
    1.0f - expf(-1.0f /
    (SAMPLE_RATE * SMOOTHING_TIME));

float signalPower = 0.0f;
bool signalDetected = false;

// ============================================================
// DEBUG CONFIGURATION
// ============================================================

constexpr uint32_t DEBUG_INTERVAL_MS = 250;

uint16_t minADC = 4095;
uint16_t maxADC = 0;

uint32_t samplesProcessed = 0;
uint32_t timingOverruns = 0;

uint32_t nextSampleUs = 0;
uint32_t lastDebugMs = 0;

// ============================================================
// RESET FILTER STATE
// ============================================================

void resetFilterState() {

    for (int i = 0; i < FILTER_SIZE; i++) {
        x[i] = 0.0f;
        y[i] = 0.0f;
    }

    signalPower = 0.0f;
    signalDetected = false;

    // Default to lamp absent while new filter settles.
    digitalWrite(LED_PIN, HIGH);
}

// ============================================================
// SELECT FILTER
// ============================================================

void selectFilter(int index) {

    if (index < 0 || index >= NUM_FILTERS) {
        Serial.println("Invalid filter selection.");
        return;
    }

    activeFilter = index;

    resetFilterState();

    Serial.println();
    Serial.println("====================================");
    Serial.printf("Selected Filter %d\n", index + 1);
    Serial.println("====================================");

    Serial.printf("Name: %s\n", filters[index].name);

    Serial.printf("Frequency range: %.0f - %.0f Hz\n",
                  filters[index].lowFreq,
                  filters[index].highFreq);

    Serial.printf("Sampling rate: %lu Hz\n", SAMPLE_RATE);

    Serial.println("Filter history reset.");
    Serial.println();

    minADC = 4095;
    maxADC = 0;
    samplesProcessed = 0;
    timingOverruns = 0;

    // Restart sample schedule.
    nextSampleUs = micros();
}

// ============================================================
// DISPLAY COMMAND MENU
// ============================================================

void printMenu() {

    Serial.println();
    Serial.println("========== IR FILTER MENU ==========");

    for (int i = 0; i < NUM_FILTERS; i++) {

        Serial.printf(
            "%d - %s (%.0f - %.0f Hz)\n",
            i + 1,
            filters[i].name,
            filters[i].lowFreq,
            filters[i].highFreq
        );
    }

    Serial.println();
    Serial.println("Enter 1, 2, or 3 to change filters.");
    Serial.println("Enter ? to display this menu.");
    Serial.println("====================================");
    Serial.println();
}

// ============================================================
// SERIAL COMMAND HANDLER
// ============================================================

void handleSerialCommands() {

    while (Serial.available() > 0) {

        char command = Serial.read();

        switch (command) {

            case '1':
                selectFilter(0);
                break;

            case '2':
                selectFilter(1);
                break;

            case '3':
                selectFilter(2);
                break;

            case '?':
                printMenu();
                break;

            default:
                // Ignore newline / carriage return.
                break;
        }
    }
}

// ============================================================
// DIGITAL IIR FILTER
// ============================================================

float applyFilter(float input) {

    const FilterConfig& filter = filters[activeFilter];

    // Shift old input/output history.
    for (int i = FILTER_SIZE - 1; i > 0; i--) {
        x[i] = x[i - 1];
        y[i] = y[i - 1];
    }

    x[0] = input;

    // Difference equation:
    //
    // y[n] = b0*x[n] + ... + bN*x[n-N]
    //      - a1*y[n-1] - ... - aN*y[n-N]

    float output = 0.0f;

    for (int i = 0; i < FILTER_SIZE; i++) {
        output += filter.num[i] * x[i];
    }

    for (int i = 1; i < FILTER_SIZE; i++) {
        output -= filter.den[i] * y[i];
    }

    // Normalize by a0.
    output /= filter.den[0];

    y[0] = output;

    return output;
}

// ============================================================
// ADC SAMPLING AND DETECTION
// ============================================================

void processSample() {

    uint16_t rawADC = analogRead(IR_PIN);

    samplesProcessed++;

    if (rawADC < minADC) minADC = rawADC;
    if (rawADC > maxADC) maxADC = rawADC;

    // Normalize a 12-bit ADC sample.
    //
    // Approximately -1 to +1.
    // The band-pass filter rejects the DC offset.

    float input =
        (static_cast<float>(rawADC) - 2048.0f)
        / 2048.0f;

    // Apply selected band-pass filter.
    float filtered = applyFilter(input);

    // Estimate filtered signal power.
    signalPower += RMS_ALPHA *
        (filtered * filtered - signalPower);

    if (signalPower < 0.0f) {
        signalPower = 0.0f;
    }

    // Lamp detection with hysteresis.
    if (signalPower >=
        PRESENT_THRESHOLD * PRESENT_THRESHOLD) {

        signalDetected = true;

    } else if (signalPower <=
        ABSENT_THRESHOLD * ABSENT_THRESHOLD) {

        signalDetected = false;
    }

    digitalWrite(LED_PIN, signalDetected ? LOW : HIGH);
}

// ============================================================
// DEBUG PRINTING
// ============================================================

void printDebug() {

    float rms = sqrtf(signalPower);

    Serial.printf(
        "F%d | ADC=%u..%u | RMS=%.5f | "
        "Lamp=%s | LED=%s | Samples=%lu | Overruns=%lu\n",
        activeFilter + 1,
        minADC,
        maxADC,
        rms,
        signalDetected ? "YES" : "NO",
        static_cast<unsigned long>(samplesProcessed),
        static_cast<unsigned long>(timingOverruns)
    );

    minADC = 4095;
    maxADC = 0;

    samplesProcessed = 0;
    timingOverruns = 0;
}

// ============================================================
// SETUP
// ============================================================

void setup() {

    Serial.begin(115200);
    delay(500);

    pinMode(IR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, HIGH);

    analogReadResolution(12);
    analogSetPinAttenuation(IR_PIN, ADC_11db);

    Serial.println();
    Serial.println("ESP32 IR Receiver");
    Serial.println("GPIO32 = ADC Input");
    Serial.println("GPIO12 = LED Output");

    resetFilterState();

    printMenu();

    selectFilter(0);

    lastDebugMs = millis();
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

    // Allow interactive filter selection.
    handleSerialCommands();

    // Fixed-rate ADC sampling.
    uint32_t now = micros();

    if ((int32_t)(now - nextSampleUs) >= 0) {

        int32_t lateness =
            (int32_t)(now - nextSampleUs);

        nextSampleUs += SAMPLE_PERIOD_US;

        if (lateness >= (int32_t)SAMPLE_PERIOD_US) {

            timingOverruns++;

            // Recover schedule rather than repeatedly
            // processing stale sample deadlines.
            nextSampleUs = micros() + SAMPLE_PERIOD_US;
        }

        processSample();
    }

    // Debug printing outside ADC processing.
    if (millis() - lastDebugMs >= DEBUG_INTERVAL_MS) {

        lastDebugMs = millis();

        printDebug();
    }
}
