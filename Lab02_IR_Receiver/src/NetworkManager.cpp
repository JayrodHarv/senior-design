#include "NetworkManager.h"
#include "Config.h"

#include <WiFi.h>

void NetworkManager::begin()
{
    Serial.println("[WiFi] Initializing...");

    WiFi.mode(WIFI_STA);

    wifiManager_.setConfigPortalBlocking(false);
    wifiManager_.setCaptivePortalEnable(true);
    wifiManager_.setWiFiAutoReconnect(true);

    wifiManager_.autoConnect(
        Config::AP_NAME,
        Config::AP_PASSWORD
    );

    wasConnected_ = isConnected();
}

void NetworkManager::update()
{
    /*
     * Required when WiFiManager is operating
     * in non-blocking mode.
     */
    wifiManager_.process();

    bool connected = isConnected();

    // Detect a new connection.
    if (connected && !wasConnected_)
    {
        Serial.println("[WiFi] Connected");

        WiFi.setAutoReconnect(true);

        printConnectionStatus();
    }

    // Detect loss of Wi-Fi.
    if (!connected && wasConnected_)
    {
        Serial.println("[WiFi] Connection lost");
    }

    wasConnected_ = connected;
}

bool NetworkManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::isConfigPortalActive()
{
    return wifiManager_.getConfigPortalActive();
}

void NetworkManager::startConfigPortal()
{
    if (wifiManager_.getConfigPortalActive())
    {
        return;
    }

    Serial.println(
        "[WiFi] Entering configuration mode..."
    );


    WiFi.setAutoReconnect(false);

    wifiManager_.disconnect();


    wifiManager_.startConfigPortal(
        Config::AP_NAME,
        Config::AP_PASSWORD
    );


    Serial.print("[WiFi] AP: ");
    Serial.println(Config::AP_NAME);

    Serial.println(
        "[WiFi] Open http://192.168.4.1"
    );
}

void NetworkManager::resetWiFiSettings()
{
    Serial.println(
        "[WiFi] Erasing saved Wi-Fi credentials"
    );

    wifiManager_.resetSettings();

    WiFi.disconnect(true, true);

    delay(500);

    startConfigPortal();
}

void NetworkManager::printConnectionStatus()
{
    Serial.print("[WiFi] SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("[WiFi] Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
}