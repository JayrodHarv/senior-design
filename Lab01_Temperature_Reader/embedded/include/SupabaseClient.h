#pragma once

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "AppState.h"

class SupabaseClient
{
public:
    bool sendReadings(const AppState& state);

    bool updateSensorStates(AppState& state);
    bool sendPendingStateChanges(AppState& state);
    bool fetchSensorStates(AppState& state);

private:
    WiFiClientSecure readingsClient_;
    HTTPClient readingsHttp_;
    bool readingsHttpInitialized_ = false;

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