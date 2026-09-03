#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 data pin
#define ONE_WIRE_BUS 4
#define ONE_WIRE_BUS_2 5

// Button pins
// Define button pin for therm A
// Define button pin for therm B

// Create OneWire communication object
extern OneWire oneWire;
extern OneWire oneWire2;
// Connect DallasTemperature to the OneWire bus
extern DallasTemperature sensor1;
extern DallasTemperature sensor2;
// Functions
float readTemperature(DallasTemperature& sensors);
bool tempStatusCheck(DallasTemperature& sensors);

#endif