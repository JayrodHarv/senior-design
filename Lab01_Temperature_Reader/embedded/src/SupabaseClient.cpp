#include "SupabaseClient.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Secrets.h"

namespace
{
constexpr uint16_t STATE_FETCH_TIMEOUT_MS = 750;
}


// =========================================================
// CONNECTION MANAGEMENT
// =========================================================

bool SupabaseClient::initializeReadingsConnection()
{
    if (readingsHttpInitialized_)
    {
        return true;
    }

    const String url =
        String(SUPABASE_URL) +
        "/rest/v1/temperature_readings";

    readingsClient_.setInsecure();

    if (!readingsHttp_.begin(readingsClient_, url))
    {
        Serial.println(
            "[Cloud] Could not initialize readings connection"
        );

        return false;
    }

    readingsHttp_.setReuse(true);
    readingsHttpInitialized_ = true;

    return true;
}

bool SupabaseClient::initializeStateFetchConnection()
{
    if (stateFetchHttpInitialized_)
    {
        return true;
    }

    const String url =
        String(SUPABASE_URL) +
        "/rest/v1/sensor_state"
        "?select=sensor_id,enabled"
        "&order=sensor_id.asc";

    stateFetchClient_.setInsecure();

    if (!stateFetchHttp_.begin(stateFetchClient_, url))
    {
        Serial.println(
            "[Cloud] Could not initialize state connection"
        );

        return false;
    }

    stateFetchHttp_.setReuse(true);

    // Do not allow a slow status request to block loop() for seconds.
    stateFetchHttp_.setTimeout(STATE_FETCH_TIMEOUT_MS);

    stateFetchHttpInitialized_ = true;

    return true;
}

void SupabaseClient::resetReadingsConnection()
{
    readingsHttp_.end();
    readingsClient_.stop();
    readingsHttpInitialized_ = false;
}

void SupabaseClient::resetStateFetchConnection()
{
    stateFetchHttp_.end();
    stateFetchClient_.stop();
    stateFetchHttpInitialized_ = false;
}


// =========================================================
// HEADERS
// =========================================================

void SupabaseClient::addHeaders(HTTPClient& http)
{
    http.addHeader(
        "Content-Type",
        "application/json"
    );

    http.addHeader(
        "apikey",
        SUPABASE_KEY
    );

    http.addHeader(
        "Prefer",
        "return=minimal"
    );
}


// =========================================================
// SEND HISTORICAL TEMPERATURE READINGS
// =========================================================

bool SupabaseClient::sendReadings(const AppState& state)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    JsonDocument json;
    JsonArray readings = json.to<JsonArray>();

    if (
        state.sensor1.enabled &&
        state.sensor1.connected &&
        !isnan(state.sensor1.temperatureC)
    )
    {
        JsonObject reading = readings.add<JsonObject>();
        reading["sensor_id"] = 1;
        reading["temperature"] = state.sensor1.temperatureC;
    }

    if (
        state.sensor2.enabled &&
        state.sensor2.connected &&
        !isnan(state.sensor2.temperatureC)
    )
    {
        JsonObject reading = readings.add<JsonObject>();
        reading["sensor_id"] = 2;
        reading["temperature"] = state.sensor2.temperatureC;
    }

    if (readings.size() == 0)
    {
        return true;
    }

    String body;
    serializeJson(json, body);

    if (!initializeReadingsConnection())
    {
        return false;
    }

    addHeaders(readingsHttp_);

    const unsigned long start = millis();
    const int responseCode = readingsHttp_.POST(body);
    const unsigned long elapsed = millis() - start;

    Serial.print("[Cloud] Temperature upload took ");
    Serial.print(elapsed);
    Serial.println(" ms");

    const bool success =
        responseCode >= 200 &&
        responseCode < 300;

    if (success)
    {
        Serial.println(
            "[Cloud] Temperature readings uploaded"
        );
    }
    else
    {
        Serial.print(
            "[Cloud] Temperature upload failed: "
        );
        Serial.println(responseCode);

        if (responseCode > 0)
        {
            Serial.println(readingsHttp_.getString());
        }

        // Recreate the connection on the next scheduled upload.
        resetReadingsConnection();
    }

    // Do not call readingsHttp_.end() after a successful request.
    return success;
}


// =========================================================
// UPDATE CURRENT SENSOR STATES
// =========================================================

bool SupabaseClient::updateSensorStates(AppState& state)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    const bool sensor1Success =
        updateSingleSensorState(1, state.sensor1);

    const bool sensor2Success =
        updateSingleSensorState(2, state.sensor2);

    return sensor1Success && sensor2Success;
}

bool SupabaseClient::sendPendingStateChanges(AppState& state)
{
    bool success = true;

    if (state.sensor1.enabledDirty)
    {
        if (updateSingleSensorState(1, state.sensor1))
        {
            state.sensor1.enabledDirty = false;
        }
        else
        {
            success = false;
        }
    }

    if (state.sensor2.enabledDirty)
    {
        if (updateSingleSensorState(2, state.sensor2))
        {
            state.sensor2.enabledDirty = false;
        }
        else
        {
            success = false;
        }
    }

    return success;
}

bool SupabaseClient::updateSingleSensorState(
    int sensorId,
    SensorState& sensor)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    String url =
        String(SUPABASE_URL) +
        "/rest/v1/sensor_state"
        "?sensor_id=eq." +
        String(sensorId);

    // State changes are rare, so a short-lived connection keeps the
    // two continuously reused fixed-endpoint clients independent.
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    if (!http.begin(client, url))
    {
        return false;
    }

    addHeaders(http);

    JsonDocument json;
    json["enabled"] = sensor.enabled;

    String body;
    serializeJson(json, body);

    const int responseCode = http.PATCH(body);

    const bool success =
        responseCode >= 200 &&
        responseCode < 300;

    if (success)
    {
        Serial.print("[Cloud] Sensor ");
        Serial.print(sensorId);
        Serial.println(" state updated");
    }
    else
    {
        Serial.print("[Cloud] State update failed: ");
        Serial.println(responseCode);

        if (responseCode > 0)
        {
            Serial.println(http.getString());
        }
    }

    http.end();
    return success;
}


// =========================================================
// FETCH SENSOR STATES
// =========================================================

bool SupabaseClient::fetchSensorStates(AppState& state)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    String response;

    if (!fetchRequest(response))
    {
        return false;
    }

    JsonDocument json;

    const DeserializationError error =
        deserializeJson(json, response);

    if (error)
    {
        Serial.print("[Cloud] State JSON error: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray sensors = json.as<JsonArray>();

    for (JsonObject sensorJson : sensors)
    {
        const int sensorId = sensorJson["sensor_id"];
        const bool enabled = sensorJson["enabled"];

        SensorState* sensor = nullptr;

        if (sensorId == 1)
        {
            sensor = &state.sensor1;
        }
        else if (sensorId == 2)
        {
            sensor = &state.sensor2;
        }

        if (sensor == nullptr || sensor->enabledDirty)
        {
            continue;
        }

        if (sensor->enabled == enabled)
        {
            continue;
        }

        sensor->enabled = enabled;

        if (!enabled)
        {
            sensor->temperatureC = NAN;
        }

        Serial.print("[Cloud] Sensor ");
        Serial.print(sensorId);
        Serial.print(" changed remotely -> ");
        Serial.println(enabled ? "ON" : "OFF");
    }

    return true;
}

bool SupabaseClient::fetchRequest(String& response)
{
    if (!initializeStateFetchConnection())
    {
        return false;
    }

    stateFetchHttp_.addHeader(
        "apikey",
        SUPABASE_KEY
    );

    const unsigned long start = millis();
    const int responseCode = stateFetchHttp_.GET();
    const unsigned long elapsed = millis() - start;

    Serial.print("[Cloud] State fetch took ");
    Serial.print(elapsed);
    Serial.println(" ms");

    if (responseCode != HTTP_CODE_OK)
    {
        Serial.print("[Cloud] State fetch failed: ");
        Serial.println(responseCode);

        if (responseCode > 0)
        {
            Serial.println(stateFetchHttp_.getString());
        }

        // Do not recurse here. Retry on the next scheduled state check.
        resetStateFetchConnection();
        return false;
    }

    // Consume the whole response before the next request reuses the socket.
    response = stateFetchHttp_.getString();

    // Do not call stateFetchHttp_.end() after a successful request.
    return true;
}
