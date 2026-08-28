// #include <Arduino.h>

// void setup() {
  
// }

// void loop() {
//   // put your main code here, to run repeatedly:
// }

//   //what for startup
//   //ESP32 dev board espressif32
//   //main loop
//   //read DS18B20 serial data
//   //send data to http server

//   //busy wait when switch is off


  #include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 data pin
#define ONE_WIRE_BUS 4

// Create OneWire communication object
OneWire oneWire(ONE_WIRE_BUS);

// Connect DallasTemperature to the OneWire bus
DallasTemperature sensors(&oneWire);

void setup()
{
    // Start Serial Monitor
    Serial.begin(115200);

    // Start the temperature sensor
    sensors.begin();

    Serial.println("DS18B20 Temperature Sensor");
    Serial.println("--------------------------");
}

void loop()
{
    // Ask the DS18B20 to measure the temperature
    sensors.requestTemperatures();

    // Get temperature from the first sensor
    float temperatureC = sensors.getTempCByIndex(0);

    // Print temperature
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");

    // Wait 2 seconds
    delay(2000);
}