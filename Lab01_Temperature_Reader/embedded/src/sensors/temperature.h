#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 data pin
#define ONE_WIRE_BUS 4
#define ONE_WIRE_BUS_2 5

// Button pins
#define ButtonPin1 12 // Pin for button 1
#define ButtonPin2 13 // Pin for button 2

extern bool temperatureSensor1Enabled; // Global variable to track the state of the temperature sensor
extern bool temperatureSensor2Enabled; // Global variable to track the state of the temperature sensor

// Create OneWire communication object
extern OneWire oneWire;
extern OneWire oneWire2;
// Connect DallasTemperature to the OneWire bus
extern DallasTemperature sensor1;
extern DallasTemperature sensor2;
// Functions
float readTemperature(DallasTemperature& sensors);
bool tempStatusCheck(DallasTemperature& sensors, bool sensorEnabled);

#endif