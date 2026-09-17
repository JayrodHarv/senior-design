#include "ButtonManager.h"
#include "Config.h"

ButtonManager::ButtonManager(AppState& state)
    : state_(state)
{
}

bool ButtonManager::DebouncedButton::sample(bool down, unsigned long now)
{
    if (down != rawDown)
    {
        rawDown = down;
        changedAt = now;
    }

    if (stableDown == rawDown ||
        now - changedAt < Config::BUTTON_DEBOUNCE_MS)
    {
        return false;
    }

    stableDown = rawDown;
    return stableDown; // One event per stable press; stable release rearms it.
}

void ButtonManager::begin()
{
    pinMode(Config::BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(Config::BUTTON_2_PIN, INPUT_PULLUP);

    // Sample in the ESP timer task so synchronous HTTP and temperature
    // conversions cannot stretch a short bounce into a second press.
    esp_timer_create_args_t args = {};
    args.callback = &ButtonManager::sampleButtons;
    args.arg = this;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "buttons";
    ESP_ERROR_CHECK(esp_timer_create(&args, &sampleTimer_));
    ESP_ERROR_CHECK(esp_timer_start_periodic(sampleTimer_, 5000));

    Serial.println("[Buttons] Debounced sampling initialized");
}

void ButtonManager::sampleButtons(void* context)
{
    static_cast<ButtonManager*>(context)->sampleButtons();
}

void ButtonManager::sampleButtons()
{
    const unsigned long now = millis();
    const bool pressed1 = button1_.sample(
        digitalRead(Config::BUTTON_1_PIN) == LOW, now);
    const bool pressed2 = button2_.sample(
        digitalRead(Config::BUTTON_2_PIN) == LOW, now);

    if (button1_.stableDown && button2_.stableDown)
    {
        if (!bothButtonsDown_)
        {
            bothButtonsDown_ = true;
            bothButtonsPressedSince_ = now;
        }

        if (!networkConfigTriggered_ &&
            now - bothButtonsPressedSince_ >= Config::NETWORK_CONFIG_HOLD_MS)
        {
            networkConfigTriggered_ = true;
            portENTER_CRITICAL(&eventMux_);
            networkConfigRequested_ = true;
            portEXIT_CRITICAL(&eventMux_);
        }
        return;
    }

    bothButtonsDown_ = false;
    networkConfigTriggered_ = false;

    // Preserve the net effect of presses while loop() is busy. Only loop()
    // changes sensor state, avoiding races with cloud uploads and reads.
    portENTER_CRITICAL(&eventMux_);
    if (pressed1) pendingToggles_ ^= 1;
    if (pressed2) pendingToggles_ ^= 2;
    portEXIT_CRITICAL(&eventMux_);
}

uint8_t ButtonManager::update()
{
    portENTER_CRITICAL(&eventMux_);
    const uint8_t toggles = pendingToggles_;
    pendingToggles_ = 0;
    portEXIT_CRITICAL(&eventMux_);

    if (toggles & 1) toggleSensor(state_.sensor1, "Sensor 1");
    if (toggles & 2) toggleSensor(state_.sensor2, "Sensor 2");
    return toggles;
}

bool ButtonManager::consumeNetworkConfigRequest()
{
    portENTER_CRITICAL(&eventMux_);
    const bool requested = networkConfigRequested_;
    networkConfigRequested_ = false;
    portEXIT_CRITICAL(&eventMux_);

    if (requested) Serial.println("[Buttons] Wi-Fi setup requested");
    return requested;
}


void ButtonManager::toggleSensor(
    SensorState& sensor,
    const char* name)
{
    sensor.enabled =
        !sensor.enabled;

    sensor.enabledDirty =
        true;


    if (!sensor.enabled)
    {
        sensor.temperatureC =
            NAN;
    }


    Serial.print("[Buttons] ");
    Serial.print(name);
    Serial.print(" -> ");

    Serial.println(
        sensor.enabled
            ? "ON"
            : "OFF"
    );
}
