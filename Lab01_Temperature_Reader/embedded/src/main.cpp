//   //what for startup
//   //ESP32 dev board espressif32
//   //main loop
//   //read DS18B20 serial data
//   //send data to http server

//   //busy wait when switch is off

#include "network/network.h"
#include "sensors/temperature.h"
#include <LiquidCrystal.h>
#include <Arduino.h>

// Initialize the LCD with the appropriate pins
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

void setup() {

    // Start Serial Monitor
    Serial.begin(115200);

    // Connect to wifi
    setupNetwork();

    // Start the temperature sensor
    sensor1.begin();
    sensor2.begin();

    Serial.println("DS18B20 Temperature Sensor");
    Serial.println("--------------------------");

    // Initialize lcd display
     lcd.begin(16, 2);

    lcd.setCursor(0, 0);
    lcd.print("Hello!");

    lcd.setCursor(0, 1);
    lcd.print("LCD Test");
    // Connect to wifi
    //connectWiFi();

    // Update initial device state on database
    //applySensorState();
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

        //needs tied to the unique sensor id
        //will be number 1
        if(tempStatusCheck(sensor1) == false){
            //display to lcd that no device is connected
            lcd.setCursor(0, 0);
            lcd.print("No Device");
        } else {
            float temperature = readTemperature(sensor1);
            uploadTemperature(temperature);
            lcd.setCursor(0, 0);
            lcd.print("Temp: " + String(temperature) + " C");
        }
        
        //needs tied to the unique sensor id
        //will be number 2
        if(tempStatusCheck(sensor2) == false){
            //display to lcd that no device is connected
            lcd.setCursor(0, 1);
            lcd.print("No Device");           
        } else {
            float temperature = readTemperature(sensor2);
            uploadTemperature(temperature);
            lcd.setCursor(0, 1);
            lcd.print("Temp: " + String(temperature) + " C");
        }
        
    }
   
}