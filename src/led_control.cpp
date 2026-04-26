#include <esp_log.h>
#include "pin_config.h"
#include "led_control.h"

static const char* TAG = "led_control";

static constexpr uint8_t kRgbLedCount = 4;
static uint8_t s_last_values[kRgbLedCount * 3] = {0};

void setupRgbLed() {
    ESP_LOGW(TAG, "[C] WS2812 driver temporaneamente disabilitato (PIN=%d)", PIN_RGB_LED_CFG);
}

void applyLedValues(const uint8_t* values) {
    for (uint8_t i = 0; i < kRgbLedCount * 3; ++i) {
        s_last_values[i] = values[i];
    }
    ESP_LOGD(TAG, "[C] LED values buffered (driver disabilitato)");
}

void ledOff() {
    for (uint8_t i = 0; i < kRgbLedCount * 3; ++i) {
        s_last_values[i] = 0;
    }
    ESP_LOGD(TAG, "[C] LED OFF (driver disabilitato)");
}
