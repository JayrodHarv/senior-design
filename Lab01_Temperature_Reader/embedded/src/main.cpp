//   //what for startup
//   //ESP32 dev board espressif32
//   //main loop
//   //read DS18B20 serial data
//   //send data to http server

//   //busy wait when switch is off

#include "network/network.h"
#include "sensors/temperature.h"

#include <Arduino.h>

void setup() {

    // Start Serial Monitor
    Serial.begin(115200);

    // Connect to wifi
    setupNetwork();

    // Start the temperature sensor
    sensors.begin();

    Serial.println("DS18B20 Temperature Sensor");
    Serial.println("--------------------------");

    // Initialize lcd display

    // Update initial device state on database
    applySensorState();
}

void loop() {

    //button logic to turn off temperature sensor

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
    //update lcd display with temperature
}