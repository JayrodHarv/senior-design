#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>

// Networking functions
void setupNetwork();
void checkCommands();
void uploadTemperature(float temperature);
void applySensorState();
void markCommandProcessed(long commandId);

// Shared state from networking.cpp
extern unsigned long lastTemperatureUpload;
extern unsigned long lastCommandCheck;
extern unsigned long TEMPERATURE_INTERVAL;
extern unsigned long COMMAND_INTERVAL;
extern bool temperatureSensorEnabled;

#endif