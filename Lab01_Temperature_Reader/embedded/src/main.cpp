
#include "network/network.h"
#include "sensors/temperature.h"
#include <LiquidCrystal.h>
#include <Arduino.h>

// Initialize the LCD with the appropriate pins
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

volatile bool button1Interrupt = false; // Flag to indicate button 1 interrupt
volatile bool button2Interrupt = false; // Flag to indicate button 2 interrupt

void IRAM_ATTR handleButton1Interrupt() {
    button1Interrupt = true; // Set the flag when button 1 is pressed
}

void IRAM_ATTR handleButton2Interrupt() {
    button2Interrupt = true; // Set the flag when button 2 is pressed
}


void setup() {
    pinMode(ButtonPin1, INPUT_PULLUP); // Set button pin 1 as input with pull-up resistor
    pinMode(ButtonPin2, INPUT_PULLUP); // Set button pin 2 as input
    // Start Serial Monitor
    Serial.begin(115200);

    attachInterrupt(digitalPinToInterrupt(ButtonPin1), handleButton1Interrupt, FALLING); // Attach interrupt for button 1
    attachInterrupt(digitalPinToInterrupt(ButtonPin2), handleButton2Interrupt, FALLING); // Attach interrupt for button 2

    // Connect to wifi
    // setupNetwork();

    // Start the temperature sensor
    sensor1.begin();
    sensor2.begin();

    Serial.println("DS18B20 Temperature Sensor");
    Serial.println("--------------------------");

    // Initialize lcd display
     lcd.begin(16, 2);
   
    // Update initial device state on database
    //applySensorState();
}

void loop() {

    bool rawButton1State = digitalRead(ButtonPin1);
    bool rawButton2State = digitalRead(ButtonPin2);
    
   if(button1Interrupt) {
        button1Interrupt = false;

        temperatureSensor1Enabled = !temperatureSensor1Enabled;

        Serial.println("Button 1 pressed");

        if(temperatureSensor1Enabled) {
            Serial.println("Sensor 1 enabled");
        } else {
            Serial.println("Sensor 1 disabled");
        }
    }

    if(button2Interrupt) {
        button2Interrupt = false;

        temperatureSensor2Enabled = !temperatureSensor2Enabled;

        Serial.println("Button 2 pressed");

        if(temperatureSensor2Enabled) {
            Serial.println("Sensor 2 enabled");
        } else {
            Serial.println("Sensor 2 disabled");
        }
    }

    if(temperatureSensor1Enabled) {
        //needs tied to the unique sensor id
        //will be number 1
        if(tempStatusCheck(sensor1, temperatureSensor1Enabled) == false){
            //display to lcd that no device is connected
            lcd.setCursor(0, 0);
            lcd.print("No Device         ");
        } else {
            float temperature = readTemperature(sensor1);
            uploadTemperature(temperature);
            lcd.setCursor(0, 0);
            lcd.print("Temp: " + String(temperature) + " C       ");
            //upload temp to database
        }
    } else {
        lcd.setCursor(0, 0);
        lcd.print("Sensor 1 Disabled     ");
    }
       

    if(temperatureSensor2Enabled) {

        if(tempStatusCheck(sensor2, temperatureSensor2Enabled) == false) {

            lcd.setCursor(0, 1);
            lcd.print("No Device       ");

        } else {

            float temperature = readTemperature(sensor2);
            uploadTemperature(temperature);

            lcd.setCursor(0, 1);
            lcd.print("Temp: " + String(temperature) + " C       ");
        }

    } else {

        lcd.setCursor(0, 1);
        lcd.print("Sensor 2 Disabled");
    }
    
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
 
    }
   
}