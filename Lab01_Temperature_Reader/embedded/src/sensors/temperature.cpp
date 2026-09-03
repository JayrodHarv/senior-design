#include "temperature.h"
//figure out how to make the sensor save button status for checks
bool tempStatusCheck(DallasTemperature& sensors, bool sensorEnabled) {
    if(!sensorEnabled) {
        Serial.println("Sensor is disabled");
        return false; // Return false if the sensor is disabled
    }
    if(sensors.getTempCByIndex(0) == DEVICE_DISCONNECTED_C) {
        Serial.println("No Device Connected");
        return false; // Return false to indicate device disconnected or device disabled
    }
    return true; // Return true if the sensor is enabled and connected
}

//Takes sensors as a parameter to read the temperature from the specified sensor
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
//normally open? or normally closed?
bool temperatureSensor1Enabled = true; // Global variable to track the state of the temperature sensor
bool temperatureSensor2Enabled = true; // Global variable to track the state of the temperature sensor
