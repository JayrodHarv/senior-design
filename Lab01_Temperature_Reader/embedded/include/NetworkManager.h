#pragma once

#include <Arduino.h>
#include <WiFiManager.h>

class NetworkManager
{
public:
    void begin();
    void update();

    bool isConnected() const;
    bool isConfigPortalActive();

    void startConfigPortal();
    void resetWiFiSettings();

private:
    WiFiManager wifiManager_;

    bool wasConnected_ = false;

    void printConnectionStatus();
};