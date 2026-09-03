#include "temperature.h"
//figure out how to make the sensor save button status for checks
bool tempStatusCheck(DallasTemperature& sensors) {
    if(sensors.getTempCByIndex(0) == DEVICE_DISCONNECTED_C) {
        Serial.println("No Device Connected");
        return false; // Return false to indicate device disconnected or device disabled
    }
}



float readTemperature(DallasTemperature& sensors) {
    // Ask the DS18B20 to measure the temperature
    sensors.requestTemperatures();

    // Get temperature from sensor
    float TemperatureC = sensors.getTempCByIndex(0);
    // Print temperature
    Serial.print("Temperature: ");
    Serial.print(TemperatureC);
    Serial.println(" °C");
    
    return TemperatureC;
}

// Define the sensor objects here to provide a single definition
OneWire oneWire(ONE_WIRE_BUS);
OneWire oneWire2(ONE_WIRE_BUS_2);
DallasTemperature sensor1(&oneWire);
DallasTemperature sensor2(&oneWire2);
int senorButtonState = 1; // Initialize the button state to HIGH (not pressed)
