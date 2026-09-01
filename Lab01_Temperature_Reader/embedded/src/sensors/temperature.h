#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 data pin
#define ONE_WIRE_BUS 4

// Create OneWire communication object
extern OneWire oneWire;

// Connect DallasTemperature to the OneWire bus
extern DallasTemperature sensors;

// Functions
float readTemperature();

#endif