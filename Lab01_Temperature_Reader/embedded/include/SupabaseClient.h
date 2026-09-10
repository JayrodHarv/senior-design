#pragma once

#include <HTTPClient.h>

#include "AppState.h"

class SupabaseClient
{
public:
    // Store historical temperature readings.
    bool sendReadings(const AppState& state);

    // Upload current sensor status.
    // Also uploads physical-button changes.
    bool updateSensorStates(AppState& state);

    bool sendPendingStateChanges(AppState& state);

    // Read enabled states changed by the web UI.
    bool fetchSensorStates(AppState& state);

private:
    bool updateSingleSensorState(
        int sensorId,
        SensorState& sensor
    );

    bool fetchRequest(
        String& response
    );

    void addHeaders(
        HTTPClient& http
    );
};