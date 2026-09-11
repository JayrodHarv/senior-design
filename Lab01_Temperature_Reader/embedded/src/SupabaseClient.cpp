#include "SupabaseClient.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "Secrets.h"


// =========================================================
// HEADERS
// =========================================================

void SupabaseClient::addHeaders(
    HTTPClient& http)
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

bool SupabaseClient::sendReadings(
    const AppState& state)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    // -----------------------------------------------------
    // Build JSON array
    //
    // [
    //   {
    //     "sensor_id": 1,
    //     "temperature": 23.5
    //   },
    //   {
    //     "sensor_id": 2,
    //     "temperature": 22.1
    //   }
    // ]
    // -----------------------------------------------------

    JsonDocument json;

    JsonArray readings =
        json.to<JsonArray>();


    // Sensor 1
    if (
        state.sensor1.enabled &&
        state.sensor1.connected &&
        !isnan(state.sensor1.temperatureC)
    )
    {
        JsonObject reading =
            readings.add<JsonObject>();

        reading["sensor_id"] = 1;

        reading["temperature"] =
            state.sensor1.temperatureC;
    }


    // Sensor 2
    if (
        state.sensor2.enabled &&
        state.sensor2.connected &&
        !isnan(state.sensor2.temperatureC)
    )
    {
        JsonObject reading =
            readings.add<JsonObject>();

        reading["sensor_id"] = 2;

        reading["temperature"] =
            state.sensor2.temperatureC;
    }


    // Nothing valid to upload.
    if (readings.size() == 0)
    {
        return true;
    }


    String body;

    serializeJson(
        json,
        body
    );


    // -----------------------------------------------------
    // Send request
    // -----------------------------------------------------

    String url =
        String(SUPABASE_URL) +
        "/rest/v1/temperature_readings";


    if (!readingsHttpInitialized_)
    {
        readingsClient_.setInsecure();

        if (!readingsHttp_.begin(
                readingsClient_,
                url))
        {
            Serial.println(
                "[Cloud] Could not start readings request"
            );

            return false;
        }

        readingsHttp_.setReuse(true);

        readingsHttpInitialized_ = true;
    }

    addHeaders(readingsHttp_);

    unsigned long start = millis();

    int responseCode =
        readingsHttp_.POST(body);

    Serial.print("[Cloud] Upload took ");
    Serial.print(millis() - start);
    Serial.println(" ms");


    bool success =
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

        Serial.println(
            responseCode
        );

        Serial.println(
            readingsHttp_.getString()
        );
    }

    return success;
}


// =========================================================
// UPDATE CURRENT SENSOR STATES
// =========================================================

bool SupabaseClient::updateSensorStates(
    AppState& state)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }


    bool sensor1Success =
        updateSingleSensorState(
            1,
            state.sensor1
        );


    bool sensor2Success =
        updateSingleSensorState(
            2,
            state.sensor2
        );


    return
        sensor1Success &&
        sensor2Success;
}

bool SupabaseClient::sendPendingStateChanges(
    AppState& state)
{
    bool success = true;

    if (state.sensor1.enabledDirty)
    {
        if (updateSingleSensorState(
                1,
                state.sensor1))
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
        if (updateSingleSensorState(
                2,
                state.sensor2))
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


// =========================================================
// UPDATE ONE SENSOR STATE
// =========================================================

bool SupabaseClient::updateSingleSensorState(
    int sensorId,
    SensorState& sensor)
{
    String url =
        String(SUPABASE_URL) +
        "/rest/v1/sensor_state";

    url +=
        "?sensor_id=eq." +
        String(sensorId);

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    if (!http.begin(client, url))
    {
        return false;
    }

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

    JsonDocument json;

    json["enabled"] =
        sensor.enabled;

    String body;

    serializeJson(
        json,
        body
    );

    int responseCode =
        http.PATCH(body);

    bool success =
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
    }

    http.end();

    return success;
}


// =========================================================
// FETCH SENSOR STATES
// =========================================================

bool SupabaseClient::fetchSensorStates(
    AppState& state)
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


    DeserializationError error =
        deserializeJson(
            json,
            response
        );


    if (error)
    {
        Serial.print(
            "[Cloud] State JSON error: "
        );

        Serial.println(
            error.c_str()
        );

        return false;
    }


    JsonArray sensors =
        json.as<JsonArray>();


    for (JsonObject sensorJson : sensors)
    {
        int sensorId =
            sensorJson["sensor_id"];

        bool enabled =
            sensorJson["enabled"];


        SensorState* sensor = nullptr;


        if (sensorId == 1)
        {
            sensor = &state.sensor1;
        }
        else if (sensorId == 2)
        {
            sensor = &state.sensor2;
        }


        if (sensor == nullptr)
        {
            continue;
        }


        /*
         * If we have a physical button change waiting
         * to upload, don't overwrite it with an older
         * cloud value.
         */
        if (!sensor->enabledDirty)
        {
            if (sensor->enabled != enabled)
            {
                sensor->enabled = enabled;

                if (!enabled)
                {
                    sensor->temperatureC =
                        NAN;
                }

                Serial.print(
                    "[Cloud] Sensor "
                );

                Serial.print(
                    sensorId
                );

                Serial.print(
                    " changed remotely -> "
                );

                Serial.println(
                    enabled
                        ? "ON"
                        : "OFF"
                );
            }
        }
    }


    return true;
}


// =========================================================
// GET CURRENT SENSOR STATE
// =========================================================

bool SupabaseClient::fetchRequest(
    String& response)
{
    String url =
        String(SUPABASE_URL) +
        "/rest/v1/sensor_state"
        "?select=sensor_id,enabled"
        "&order=sensor_id.asc";


    WiFiClientSecure client;

    client.setInsecure();


    HTTPClient http;

    if (!http.begin(client, url))
    {
        return false;
    }


    http.addHeader(
        "apikey",
        SUPABASE_KEY
    );


    int responseCode =
        http.GET();


    if (responseCode != 200)
    {
        Serial.print(
            "[Cloud] State fetch failed: "
        );

        Serial.println(
            responseCode
        );

        Serial.println(
            http.getString()
        );

        http.end();

        return false;
    }


    response =
        http.getString();


    http.end();

    return true;
}