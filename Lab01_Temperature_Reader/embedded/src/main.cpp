#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "AppState.h"
#include "NetworkManager.h"
#include "SensorManager.h"
#include "ButtonManager.h"
#include "DisplayManager.h"
#include "SupabaseClient.h"
#include "Config.h"

AppState appState;
NetworkManager network;
ButtonManager buttons(appState);
DisplayManager display;

// Only copies cross task boundaries. The main loop owns the live state and LCD.
struct BackgroundWork
{
    AppState state;
    uint32_t revision1;
    uint32_t revision2;
    bool readSensors;
    bool fetchStates;
};

QueueHandle_t workQueue;
QueueHandle_t resultQueue;
uint32_t revision1 = 0;
uint32_t revision2 = 0;
bool workInProgress = false;
unsigned long lastSensorRead = 0;
unsigned long lastStateCheck = 0;
unsigned long lastStateAttempt = 0;

void backgroundTask(void*)
{
    AppState workerState;
    SensorManager sensors(workerState);
    SupabaseClient cloud;
    sensors.begin();

    BackgroundWork work;
    for (;;)
    {
        xQueueReceive(workQueue, &work, portMAX_DELAY);
        workerState = work.state;

        cloud.sendPendingStateChanges(workerState);
        if (work.readSensors)
        {
            sensors.read();
            cloud.sendReadings(workerState);
        }
        if (work.fetchStates)
        {
            cloud.fetchSensorStates(workerState);
        }

        work.state = workerState;
        xQueueSend(resultQueue, &work, portMAX_DELAY);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.println("ESP32 Temperature Monitor");

    buttons.begin();
    display.begin();
    network.begin();

    workQueue = xQueueCreate(1, sizeof(BackgroundWork));
    resultQueue = xQueueCreate(1, sizeof(BackgroundWork));
    ESP_ERROR_CHECK(workQueue && resultQueue ? ESP_OK : ESP_ERR_NO_MEM);
    const BaseType_t created = xTaskCreate(
        backgroundTask, "sensor-cloud", 12288, nullptr, 1, nullptr);
    ESP_ERROR_CHECK(created == pdPASS ? ESP_OK : ESP_ERR_NO_MEM);
}

void loop()
{
    const uint8_t toggles = buttons.update();
    if (toggles & 1) ++revision1;
    if (toggles & 2) ++revision2;

    BackgroundWork result;
    if (xQueueReceive(resultQueue, &result, 0) == pdTRUE)
    {
        // A button pressed during an HTTP request wins over its older result.
        // Keep its dirty flag so the next request sends the new state.
        if (revision1 == result.revision1) appState.sensor1 = result.state.sensor1;
        if (revision2 == result.revision2) appState.sensor2 = result.state.sensor2;
        workInProgress = false;
    }

    display.update(appState, network.isConnected());
    network.update();
    if (buttons.consumeNetworkConfigRequest())
    {
        network.startConfigPortal();
    }

    const unsigned long now = millis();
    const bool readSensors = now - lastSensorRead >= Config::SENSOR_INTERVAL_MS;
    const bool fetchStates = network.isConnected() &&
        now - lastStateCheck >= Config::STATE_CHECK_INTERVAL_MS;
    const bool sendState = network.isConnected() &&
        (appState.sensor1.enabledDirty || appState.sensor2.enabledDirty) &&
        (toggles || now - lastStateAttempt >= 1000);

    if (!workInProgress && (readSensors || fetchStates || sendState))
    {
        BackgroundWork work = {
            appState, revision1, revision2, readSensors, fetchStates
        };
        if (xQueueSend(workQueue, &work, 0) == pdTRUE)
        {
            workInProgress = true;
            if (readSensors) lastSensorRead = now;
            if (fetchStates) lastStateCheck = now;
            lastStateAttempt = now;
        }
    }

    delay(5);
}
