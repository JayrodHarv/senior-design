#include "temperature.h"

float readTemperature() {
    // Ask the DS18B20 to measure the temperature
    sensors.requestTemperatures();

    // Get temperature from the first sensor
    float temperatureC = sensors.getTempCByIndex(0);

    // Print temperature
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");
    return temperatureC;
}

// Define the sensor objects here to provide a single definition
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);