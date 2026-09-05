//   //what for startup
//   //ESP32 dev board espressif32
//   //main loop
//   //read DS18B20 serial data
//   //send data to http server

//   //cut power to esp32 dev board when switch is off.

#include "network/network.h"
#include "sensors/temperature.h"
#include <LiquidCrystal.h>
#include <Arduino.h>

// Initialize the LCD with the appropriate pins
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

bool lastRawButton1State = HIGH; // Assume button is not pressed initially
// bool lastButton2State = HIGH; // Assume button is not pressed initially

bool stableButton1State = HIGH; // Stable state of button 1

unsigned long lastDebounceTime = 0; // The last time the button input was toggled
const unsigned long debounceDelay = 50; // The debounce time; increase if the output flick

void setup() {
    pinMode(ButtonPin1, INPUT_PULLUP); // Set button pin 1 as input with pull-up resistor
    pinMode(ButtonPin2, INPUT_PULLUP); // Set button pin 2 as input
    // Start Serial Monitor
    Serial.begin(115200);

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
    bool button2State = digitalRead(ButtonPin2);
    
    if(rawButton1State != lastRawButton1State) {
        lastDebounceTime = millis(); // Reset the debouncing timer
    }
    
    if((millis() - lastDebounceTime) > debounceDelay) {
        if(rawButton1State != stableButton1State) {
            stableButton1State = rawButton1State;

            if(stableButton1State == LOW) {
                temperatureSensor1Enabled = !temperatureSensor1Enabled; // Toggle the state of sensor 1
                // Button 1 is pressed
                Serial.println("Button 1 pressed");
                if(temperatureSensor1Enabled) {
                    Serial.println("Sensor 1 enabled");
                } else {
                    Serial.println("Sensor 1 disabled");
                }
            }
        }
    }   
    lastRawButton1State = rawButton1State;

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
       
    // if(button2State == HIGH && lastButton2State == LOW) {
    //     temperatureSensor2Enabled = !temperatureSensor2Enabled; // Toggle the state of sensor 2
    //     // Button 2 is pressed
    //     Serial.println("Button 2 pressed");
    //     if(temperatureSensor2Enabled) {
    //         Serial.println("Sensor 2 enabled");
    //     } else {
    //         Serial.println("Sensor 2 disabled");
    //     }
    //     delay(50); // Debounce delay
    // }

    // lastButton2State = button2State;

    // if(temperatureSensor2Enabled){
    //     //needs tied to the unique sensor id
    //     //will be number 2
    //     if(tempStatusCheck(sensor2, temperatureSensor2Enabled) == false){
    //         //display to lcd that no device is connected
    //         lcd.setCursor(0, 1);
    //         lcd.print("No Device");           
    //     } else {
    //         float temperature = readTemperature(sensor2);
    //         uploadTemperature(temperature);
    //         lcd.setCursor(0, 1);
    //         lcd.print("Temp: " + String(temperature) + " C");
    //         //upload temp to database
    //     }
    // }
   
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