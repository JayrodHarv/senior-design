#include "SensorManager.h"

SensorManager::SensorManager(AppState& state)
    : state_(state),
      oneWire1_(Config::SENSOR_1_PIN),
      oneWire2_(Config::SENSOR_2_PIN),
      sensor1_(&oneWire1_),
      sensor2_(&oneWire2_)
{
}

void SensorManager::begin()
{
    sensor1_.begin();
    sensor2_.begin();

    // DS18B20 resolution:
    //
    // 9 bit  = 0.5 C
    // 10 bit = 0.25 C
    // 11 bit = 0.125 C
    // 12 bit = 0.0625 C
    //
    // 10-bit gives a good balance between speed and precision.

    sensor1_.setResolution(10);
    sensor2_.setResolution(10);

    Serial.println("[Sensors] Initialized");
}

void SensorManager::read()
{
    readSensor(
        sensor1_,
        state_.sensor1,
        "Sensor 1"
    );

    readSensor(
        sensor2_,
        state_.sensor2,
        "Sensor 2"
    );
}

void SensorManager::readSensor(
    DallasTemperature& sensor,
    SensorState& state,
    const char* name)
{
    // If the user turned this sensor off,
    // don't bother reading it.
    if (!state.enabled)
    {
        state.temperatureC = NAN;
        return;
    }

    sensor.requestTemperatures();

    float temperature =
        sensor.getTempCByIndex(0);

    // DallasTemperature returns DEVICE_DISCONNECTED_C
    // when communication fails.
    if (temperature == DEVICE_DISCONNECTED_C)
    {
        state.connected = false;
        state.temperatureC = NAN;

        Serial.print("[Sensors] ");
        Serial.print(name);
        Serial.println(" disconnected");

        return;
    }

    state.connected = true;
    state.temperatureC = temperature;

    Serial.print("[Sensors] ");
    Serial.print(name);
    Serial.print(": ");
    Serial.print(temperature);
    Serial.println(" C");
}