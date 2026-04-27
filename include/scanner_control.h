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

void setupScannerPins();
void initializeScanner();
#if defined(SCANNER_CONTROL_USE_SERIAL)
void scannerOn(uart_port_t scannerPort);
void scannerOff(uart_port_t scannerPort);
void scannerSerialSelfTest(uart_port_t scannerPort);
#else
void scannerOn();
void scannerOff();
#endif
void forwardScannerData(uart_port_t source, uart_port_t destination);
