#pragma once

#include <Arduino.h>

#if defined(SCANNER_CONTROL_USE_SERIAL) && defined(SCANNER_CONTROL_USE_TRIGGER)
#error "Only one scanner control mode may be defined."
#endif

#ifndef SCANNER_CONTROL_USE_SERIAL
#ifndef SCANNER_CONTROL_USE_TRIGGER
#define SCANNER_CONTROL_USE_TRIGGER
#endif
#endif

void setupScannerPins();
void initializeScanner();
#if defined(SCANNER_CONTROL_USE_SERIAL)
void scannerOn(HardwareSerial& scannerSerial);
void scannerOff(HardwareSerial& scannerSerial);
#else
void scannerOn();
void scannerOff();
#endif
void forwardScannerData(HardwareSerial& source, HardwareSerial& destination);
