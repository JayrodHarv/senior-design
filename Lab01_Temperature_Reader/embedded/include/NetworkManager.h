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

    static constexpr const char* AP_NAME =
        "TemperatureMonitor-Setup";
    
    static constexpr const char* AP_PASSWORD =
        "TempSetup123";

    void printConnectionStatus();
};