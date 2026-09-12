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
    // Keep fixed endpoints on separate persistent HTTP clients.
    WiFiClientSecure readingsClient_;
    HTTPClient readingsHttp_;
    bool readingsHttpInitialized_ = false;

    WiFiClientSecure stateFetchClient_;
    HTTPClient stateFetchHttp_;
    bool stateFetchHttpInitialized_ = false;

    bool initializeReadingsConnection();
    bool initializeStateFetchConnection();
    void resetReadingsConnection();
    void resetStateFetchConnection();

    bool updateSingleSensorState(
        int sensorId,
        SensorState& sensor
    );

    bool fetchRequest(String& response);
    void addHeaders(HTTPClient& http);
};
