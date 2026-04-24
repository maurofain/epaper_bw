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

void handleMasterSerial(HardwareSerial& source
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , HardwareSerial& scannerSerial
#endif
);
