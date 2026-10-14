#pragma once

#include <WiFiManager.h>

// Same Wi-Fi handling as Lab 1 (WiFiManager, non-blocking captive portal).
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
    void printConnectionStatus();

    WiFiManager wifiManager_;
    bool wasConnected_ = false;
};
