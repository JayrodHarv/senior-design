#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "NetworkManager.h"
#include "BeamDetector.h"
#include "IrFilter.h"
#include "SupabaseClient.h"
#include "Config.h"

NetworkManager network;
BeamDetector beam;

// Only copies cross task boundaries. The main loop owns the live beam state.
struct BackgroundWork
{
    bool broken;
    uint32_t revision;
    bool success;
};

QueueHandle_t workQueue;
QueueHandle_t resultQueue;
uint32_t revision = 0;
bool workInProgress = false;
unsigned long lastStateAttempt = 0;

bool broken = false;
bool brokenDirty = true; // write "intact" once at boot

void backgroundTask(void*)
{
    SupabaseClient cloud;

    BackgroundWork work;
    for (;;)
    {
        xQueueReceive(workQueue, &work, portMAX_DELAY);

        work.success = cloud.updateBeamState(work.broken);

        xQueueSend(resultQueue, &work, portMAX_DELAY);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.println("ESP32 IR Beam-Break Detector");

    IrFilter::begin();
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
    // BeamDetector stays "broken" until the beam is seen once, so the first
    // change after boot is always to intact and no alert is sent at boot.
    const bool changed = beam.update(IrFilter::beamPresent(), millis());
    if (changed)
    {
        broken = beam.isBroken();
        brokenDirty = true;
        ++revision;

        Serial.println(broken ? "[Beam] BROKEN" : "[Beam] Intact");
    }

    BackgroundWork result;
    if (xQueueReceive(resultQueue, &result, 0) == pdTRUE)
    {
        // A beam change during an HTTP request wins over its older result.
        // Keep the dirty flag so the next request sends the new state.
        if (result.success && revision == result.revision) brokenDirty = false;
        workInProgress = false;
    }

    network.update();

    const unsigned long now = millis();
    const bool sendState = network.isConnected() && brokenDirty &&
        (changed || now - lastStateAttempt >= 1000);

    if (!workInProgress && sendState)
    {
        BackgroundWork work = {broken, revision, false};
        if (xQueueSend(workQueue, &work, 0) == pdTRUE)
        {
            workInProgress = true;
            lastStateAttempt = now;
        }
    }

    delay(5);
}