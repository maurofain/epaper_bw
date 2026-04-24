#pragma once

#include <Arduino.h>

void setupDisplay();
void setupLvgl();
void buildUi();
void clearDisplay();
void displayText(const String& raw_text);
