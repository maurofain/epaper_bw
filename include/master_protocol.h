#pragma once

#include <driver/uart.h>
#include <cstdint>

#if defined(SCANNER_CONTROL_USE_SERIAL) && defined(SCANNER_CONTROL_USE_TRIGGER)
#error "Only one scanner control mode may be defined."
#endif

#ifndef SCANNER_CONTROL_USE_SERIAL
#ifndef SCANNER_CONTROL_USE_TRIGGER
#define SCANNER_CONTROL_USE_TRIGGER
#endif
#endif

void handleMasterSerial(uart_port_t source
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , uart_port_t scannerPort
#endif
);
