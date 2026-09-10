#include "DisplayManager.h"

#include "Config.h"

DisplayManager::DisplayManager()
    : lcd_(
          Config::LCD_RS,
          Config::LCD_EN,
          Config::LCD_D4,
          Config::LCD_D5,
          Config::LCD_D6,
          Config::LCD_D7)
{
}

void DisplayManager::begin()
{
    lcd_.begin(
        Config::LCD_COLUMNS,
        Config::LCD_ROWS
    );

    lcd_.clear();

    printLine(0, "Temp Monitor");
    printLine(1, "Starting...");

    Serial.println("[LCD] Initialized");
}

void DisplayManager::update(
    const AppState& state,
    bool wifiConnected)
{
    if (millis() - lastUpdate_ <
        Config::DISPLAY_INTERVAL_MS)
    {
        return;
    }

    lastUpdate_ = millis();

    displaySensor(
        0,
        1,
        state.sensor1
    );

    displaySensor(
        1,
        2,
        state.sensor2
    );
}

void DisplayManager::displaySensor(
    uint8_t row,
    uint8_t number,
    const SensorState& sensor)
{
    char line[17];

    if (!sensor.enabled)
    {
        snprintf(
            line,
            sizeof(line),
            "S%d OFF",
            number
        );
    }
    else if (!sensor.connected)
    {
        snprintf(
            line,
            sizeof(line),
            "S%d ERR",
            number
        );
    }
    else
    {
        snprintf(
            line,
            sizeof(line),
            "S%d ON   %5.1fC",
            number,
            sensor.temperatureC
        );
    }

    printLine(row, line);
}

void DisplayManager::printLine(
    uint8_t row,
    const char* text)
{
    lcd_.setCursor(0, row);

    char buffer[17];

    snprintf(
        buffer,
        sizeof(buffer),
        "%-16s",
        text
    );

    lcd_.print(buffer);
}