#include "ButtonManager.h"
#include "Config.h"


volatile bool ButtonManager::button1Pressed_ = false;
volatile bool ButtonManager::button2Pressed_ = false;


ButtonManager::ButtonManager(AppState& state)
    : state_(state)
{
}


void ButtonManager::begin()
{
    pinMode(
        Config::BUTTON_1_PIN,
        INPUT_PULLUP
    );

    pinMode(
        Config::BUTTON_2_PIN,
        INPUT_PULLUP
    );


    attachInterrupt(
        digitalPinToInterrupt(
            Config::BUTTON_1_PIN
        ),
        handleButton1Interrupt,
        FALLING
    );


    attachInterrupt(
        digitalPinToInterrupt(
            Config::BUTTON_2_PIN
        ),
        handleButton2Interrupt,
        FALLING
    );


    Serial.println(
        "[Buttons] Interrupts initialized"
    );
}


void ButtonManager::update()
{
    static unsigned long lastButton1Time = 0;
    static unsigned long lastButton2Time = 0;

    unsigned long now = millis();


    if (button1Pressed_)
    {
        button1Pressed_ = false;

        if (
            now - lastButton1Time >=
            Config::BUTTON_DEBOUNCE_MS
        )
        {
            lastButton1Time = now;

            toggleSensor(
                state_.sensor1,
                "Sensor 1"
            );
        }
    }


    if (button2Pressed_)
    {
        button2Pressed_ = false;

        if (
            now - lastButton2Time >=
            Config::BUTTON_DEBOUNCE_MS
        )
        {
            lastButton2Time = now;

            toggleSensor(
                state_.sensor2,
                "Sensor 2"
            );
        }
    }
}


void IRAM_ATTR ButtonManager::handleButton1Interrupt()
{
    button1Pressed_ = true;
}


void IRAM_ATTR ButtonManager::handleButton2Interrupt()
{
    button2Pressed_ = true;
}


void ButtonManager::toggleSensor(
    SensorState& sensor,
    const char* name)
{
    sensor.enabled =
        !sensor.enabled;

    sensor.enabledDirty =
        true;


    if (!sensor.enabled)
    {
        sensor.temperatureC =
            NAN;
    }


    Serial.print("[Buttons] ");
    Serial.print(name);
    Serial.print(" -> ");

    Serial.println(
        sensor.enabled
            ? "ON"
            : "OFF"
    );
}