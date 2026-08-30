#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============================================
// WIFI
// ============================================
const char* WIFI_SSID =
    "YOUR_WIFI_NAME";

const char* WIFI_PASSWORD =
    "YOUR_WIFI_PASSWORD";

// ============================================
// SUPABASE
// ============================================
const char* SUPABASE_URL =
    "https://witvuhdpcpfolevfjapq.supabase.co";

const char* SUPABASE_KEY =
    "sb_publishable_sbNdNAgtIPJu5M8eq8u4uQ_yZfwFBnN";

const char* DEVICE_ID =
    "esp32-001";

// ============================================
// TIMING
// ============================================
unsigned long lastTemperatureUpload = 0;

unsigned long lastCommandCheck = 0;

const unsigned long TEMPERATURE_INTERVAL = 1000;

const unsigned long COMMAND_INTERVAL = 250;

// ============================================
// SENSOR STATE
// ============================================
bool temperatureSensorEnabled = true;

// ============================================
// WIFI CONNECTION
// ============================================
void connectWiFi() {

    Serial.println("Connecting to WiFi...");

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    while (
        WiFi.status() != WL_CONNECTED
    ) {

        delay(500);
        Serial.print(".");
    }

    Serial.println();

    Serial.println(
        "WiFi connected!"
    );

    Serial.print(
        "IP address: "
    );

    Serial.println(
        WiFi.localIP()
    );
}

// ============================================
// SUPABASE HEADERS
// ============================================
void addSupabaseHeaders(
    HTTPClient& http
) {
    http.addHeader(
        "apikey",
        SUPABASE_KEY
    );

    http.addHeader(
        "Authorization",
        String("Bearer ") + SUPABASE_KEY
    );

    http.addHeader(
        "Content-Type",
        "application/json"
    );
}

// ============================================
// UPLOAD TEMPERATURE
// ============================================
void uploadTemperature(
    float temperature
) {

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    HTTPClient http;

    String url =
        String(SUPABASE_URL) +
        "/rest/v1/temperature_readings";

    http.begin(url);

    addSupabaseHeaders(http);

    http.addHeader(
        "Prefer",
        "return=minimal"
    );

    JsonDocument doc;

    doc["device_id"] =
        DEVICE_ID;

    doc["temperature"] =
        temperature;

    String body;

    serializeJson(
        doc,
        body
    );

    int responseCode =
        http.POST(body);

    Serial.print(
        "Temperature upload: "
    );

    Serial.println(
        responseCode
    );

    if (
        responseCode < 200 ||
        responseCode >= 300
    ) {

        Serial.println(
            http.getString()
        );
    }

    http.end();
}


// ============================================
// CHECK FOR COMMANDS
// ============================================

void checkCommands() {
    if (
        WiFi.status() != WL_CONNECTED
    ) {
        return;
    }

    HTTPClient http;

    String url =
        String(SUPABASE_URL) +
        "/rest/v1/device_commands";

    url +=
        "?device_id=eq." +
        String(DEVICE_ID);

    url +=
        "&processed_at=is.null";

    url +=
        "&order=id.asc";

    url +=
        "&limit=10";

    http.begin(url);

    addSupabaseHeaders(http);

    int responseCode =
        http.GET();

    if (
        responseCode != 200
    ) {

        Serial.print(
            "Command check failed: "
        );

        Serial.println(
            responseCode
        );

        http.end();

        return;
    }

    String response =
        http.getString();

    http.end();

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(
            doc,
            response
        );

    if (error) {

        Serial.println(
            "JSON parse failed"
        );

        return;
    }

    JsonArray commands =
        doc.as<JsonArray>();

    for (
        JsonObject command :
        commands
    ) {

        const char* commandName =
            command["command"];

        bool value =
            command["value"];

        long id =
            command["id"];

        Serial.print(
            "Received command: "
        );

        Serial.println(
            commandName
        );

        if (
            strcmp(
                commandName,
                "temperature_sensor"
            ) == 0
        ) {

            temperatureSensorEnabled =
                value;

            Serial.print(
                "Temperature sensor: "
            );

            Serial.println(
                temperatureSensorEnabled
                    ? "ON"
                    : "OFF"
            );

            applySensorState();
        }

        markCommandProcessed(
            id
        );
    }
}

// ============================================
// APPLY SENSOR STATE
// ============================================
void applySensorState() {
    /*
     * Replace this with the actual
     * hardware control.
     *
     * Example:
     *
     * digitalWrite(
     *     SENSOR_POWER_PIN,
     *     temperatureSensorEnabled
     * );
     */

    Serial.print(
        "Sensor state = "
    );

    Serial.println(
        temperatureSensorEnabled
            ? "ON"
            : "OFF"
    );
}


// ============================================
// MARK COMMAND PROCESSED
// ============================================

void markCommandProcessed(
    long commandId
) {
    HTTPClient http;

    String url =
        String(SUPABASE_URL) +
        "/rest/v1/device_commands";

    url +=
        "?id=eq." +
        String(commandId);

    http.begin(url);

    addSupabaseHeaders(http);

    http.addHeader(
        "Prefer",
        "return=minimal"
    );

    JsonDocument doc;

    doc["processed_at"] =
        "now()";

    /*
     * Supabase REST doesn't evaluate
     * "now()" as JSON.
     *
     * Instead we'll use an ISO timestamp
     * generated by Postgres via an RPC
     * in a production version.
     *
     * For this simple example we can
     * simply update using the current
     * ESP32 timestamp later.
     */

    doc["processed_at"] = millis();

    String body;

    serializeJson(
        doc,
        body
    );

    int responseCode =
        http.PATCH(body);

    Serial.print(
        "Mark command processed: "
    );

    Serial.println(
        responseCode
    );

    http.end();
}