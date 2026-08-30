//   //what for startup
//   //ESP32 dev board espressif32
//   //main loop
//   //read DS18B20 serial data
//   //send data to http server

//   //busy wait when switch is off

#include "networking.h"

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 data pin
#define ONE_WIRE_BUS 4

// Create OneWire communication object
OneWire oneWire(ONE_WIRE_BUS);

// Connect DallasTemperature to the OneWire bus
DallasTemperature sensors(&oneWire);

float readTemperature() {
    // Ask the DS18B20 to measure the temperature
    sensors.requestTemperatures();

    // Get temperature from the first sensor
    float temperatureC = sensors.getTempCByIndex(0);

    // Print temperature
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");
}

void setup() {

    // Start Serial Monitor
    Serial.begin(115200);

    // Start the temperature sensor
    sensors.begin();

    Serial.println("DS18B20 Temperature Sensor");
    Serial.println("--------------------------");

    // Connect to wifi
    connectWiFi();

    // Update initial device state on database
    applySensorState();
}

void loop() {

    unsigned long now = millis();

    // ----------------------------
    // Check commands
    // ----------------------------
    if (
        now - lastCommandCheck >=
        COMMAND_INTERVAL
    ) {
        lastCommandCheck = now;
        checkCommands();
    }

    // ----------------------------
    // Send temperature
    // ----------------------------
    if (
        temperatureSensorEnabled
        && now - lastTemperatureUpload >= TEMPERATURE_INTERVAL
    ) {

        lastTemperatureUpload = now;

        float temperature = readTemperature();

        Serial.print(
            "Temperature: "
        );

        Serial.println(
            temperature
        );

        uploadTemperature(
            temperature
        );
    }
}