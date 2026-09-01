#include "temperature.h"

float readTemperature() {
    // Ask the DS18B20 to measure the temperature
    sensors.requestTemperatures();
    sensors2.requestTemperatures();
    // Get temperature from the first sensor
    // Needs if statement to check if second sensor is connected
    float temperatureC = sensors.getTempCByIndex(0);
    // Needs if statement to check if second sensor is connected
    float temperatureC2 = sensors2.getTempCByIndex(1);

    // Print temperature
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");
    return temperatureC;
}

// Define the sensor objects here to provide a single definition
OneWire oneWire(ONE_WIRE_BUS);
OneWire oneWire2(ONE_WIRE_BUS_2);
DallasTemperature sensors(&oneWire);
DallasTemperature sensors2(&oneWire2);