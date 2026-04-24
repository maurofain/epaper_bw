#pragma once

#include <Arduino.h>

void setupDisplay();
void setupLvgl();
void buildUi();
void clearDisplay();
void displayText(const String& raw_text);
void displayText(const String& raw_text, uint8_t fontNumber, uint8_t x, uint8_t y);
