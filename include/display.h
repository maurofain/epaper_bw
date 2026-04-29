#pragma once

#include <cstdint>
#include <string>

enum class DisplayOrientation
{
    ORIENTATION_0 = 0,
    ORIENTATION_90 = 90,
    ORIENTATION_180 = 180,
    ORIENTATION_270 = 270,
};

void setupDisplay();
void setupLvgl();
void buildUi();
void clearDisplay();
void displayText(const std::string& raw_text);
void displayText(const std::string& raw_text, uint8_t fontNumber, uint8_t x, uint8_t y);
void displayJpegCentered(const char* path);
void setDisplayOrientation(DisplayOrientation orientation);
