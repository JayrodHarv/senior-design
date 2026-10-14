#include "SupabaseClient.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Config.h"
#include "Secrets.h"


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
// UPDATE CURRENT BEAM STATE
// =========================================================

bool SupabaseClient::updateBeamState(bool broken)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    String url =
        String(SUPABASE_URL) +
        "/rest/v1/beam_state"
        "?id=eq." +
        String(Config::BEAM_ROW_ID);

    // State changes are rare, so a short-lived connection avoids reusing
    // a socket the server closed while the beam sat idle.
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    if (!http.begin(client, url))
    {
        return false;
    }

    http.setTimeout(Config::HTTP_TIMEOUT_MS);

    addHeaders(http);

    JsonDocument json;
    json["broken"] = broken;

    String body;
    serializeJson(json, body);

    const int responseCode = http.PATCH(body);

    const bool success =
        responseCode >= 200 &&
        responseCode < 300;

    if (success)
    {
        Serial.print("[Cloud] Beam state updated -> ");
        Serial.println(broken ? "BROKEN" : "INTACT");
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
