#include <Arduino.h>

#include "AppState.h"

#include "NetworkManager.h"
#include "SensorManager.h"
#include "ButtonManager.h"
#include "DisplayManager.h"
#include "SupabaseClient.h"

#include "Config.h"


AppState appState;

NetworkManager network;
SensorManager sensors(appState);
ButtonManager buttons(appState);
DisplayManager display;
SupabaseClient cloud;


unsigned long lastSensorRead = 0;
unsigned long lastStateCheck = 0;

constexpr unsigned long SENSOR_INTERVAL = 1000;
constexpr unsigned long STATE_CHECK_INTERVAL = 500;


void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" ESP32 Temperature Monitor");
    Serial.println("==============================");

    sensors.begin();
    buttons.begin();
    display.begin();

    network.begin();
}


void loop()
{

    // Serial.print("ESP32 MAC Address: ");
    // Serial.println(WiFi.macAddress());

    network.update();

    buttons.update();

    if (buttons.consumeNetworkConfigRequest())
    {
        network.startConfigPortal();
    }

    display.update(
        appState,
        network.isConnected()
    );


    unsigned long now =
        millis();


    // =========================================
    // Send physical button changes
    // =========================================

    if (network.isConnected())
    {
        cloud.sendPendingStateChanges(
            appState
        );
    }


    // =========================================
    // Read sensors + upload temperature
    // =========================================

    if (
        now - lastSensorRead >=
        SENSOR_INTERVAL
    )
    {
        lastSensorRead = now;

        sensors.read();

        if (network.isConnected())
        {
            cloud.sendReadings(
                appState
            );
        }
    }


    // =========================================
    // Check for web UI state changes
    // =========================================

    if (
        network.isConnected() &&
        now - lastStateCheck >=
        STATE_CHECK_INTERVAL
    )
    {
        lastStateCheck = now;

        cloud.fetchSensorStates(
            appState
        );
    }
}