#include "NetworkManager.h"

#include <WiFi.h>

void NetworkManager::begin()
{
    Serial.println("[WiFi] Initializing WiFiManager...");

    // Explicitly use station mode.
    WiFi.mode(WIFI_STA);

    /*
     * IMPORTANT:
     *
     * Non-blocking configuration portal.
     *
     * This allows the rest of the ESP32 application
     * to continue running while the user configures
     * Wi-Fi.
     */
    wifiManager_.setConfigPortalBlocking(false);

    // Explicitly enable captive portal behavior.
    wifiManager_.setCaptivePortalEnable(true);

    wifiManager_.setWiFiAPChannel(1);

    /*
     * Automatically reconnect if the Wi-Fi
     * connection is temporarily lost.
     */
    wifiManager_.setWiFiAutoReconnect(true);

    /*
     * Start by attempting the previously saved
     * Wi-Fi network.
     *
     * If there is no saved network, or the saved
     * network cannot be reached, WiFiManager starts
     * its configuration access point.
     */
    bool connected =
        wifiManager_.autoConnect(AP_NAME);

    if (connected)
    {
        Serial.println("[WiFi] Connected using saved credentials");

        printConnectionStatus();
    }
    else
    {
        Serial.println("[WiFi] Configuration portal started");
        Serial.print("[WiFi] Connect to AP: ");
        Serial.println(AP_NAME);

        Serial.println("[WiFi] Open:");
        Serial.println("[WiFi] http://192.168.4.1");
    }

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
        Serial.println(
            "[WiFi] Configuration portal already active"
        );

        return;
    }

    Serial.println(
        "[WiFi] Starting configuration portal..."
    );

    wifiManager_.startConfigPortal(AP_NAME);

    Serial.print("[WiFi] Connect to AP: ");
    Serial.println(AP_NAME);

    Serial.println("[WiFi] Open:");
    Serial.println("[WiFi] http://192.168.4.1");
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