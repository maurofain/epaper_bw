#pragma once

#include <cstdint>
#include <string>

void setupDisplay();
void setupLvgl();
void buildUi();
void clearDisplay();
void displayText(const std::string& raw_text);
void displayText(const std::string& raw_text, uint8_t fontNumber, uint8_t x, uint8_t y);
