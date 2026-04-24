#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "pin_config.h"
#include "led_control.h"

Adafruit_NeoPixel rgb_led(4, PIN_RGB_LED_CFG, NEO_GRB + NEO_KHZ800);

void setupRgbLed() {
    rgb_led.begin();
    rgb_led.clear();
    rgb_led.show();
}

void applyLedValues(const uint8_t* values) {
    Serial.println("[DEBUG] Applying LED color values");
    for (uint8_t i = 0; i < 4; ++i) {
        const uint8_t r = values[i * 3];
        const uint8_t g = values[i * 3 + 1];
        const uint8_t b = values[i * 3 + 2];
        Serial.printf("[DEBUG] LED %u = R:%u G:%u B:%u\n", i, r, g, b);
        rgb_led.setPixelColor(i, rgb_led.Color(r, g, b));
    }
    rgb_led.show();
}

void ledOff() {
    rgb_led.clear();
    rgb_led.show();
    Serial.println("LED OFF");
}
