#ifndef NETWORK_H
#define NETWORK_H

void connectWifi();

void checkCommands();

void uploadTemperature(float temperature);

bool applySensorState();

void markCommandProcessed(long commandId);

#endif