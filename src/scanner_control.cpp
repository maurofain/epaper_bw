#include <Arduino.h>
#include "pin_config.h"
#include "scanner_control.h"

static bool scannerInitialized = false;
static String scannerBuffer = "";

#if defined(SCANNER_CONTROL_USE_SERIAL)
static const uint8_t START_SCAN_CMD[] = {0x01, 'T', 0x04};
static const uint8_t STOP_SCAN_CMD[] = {0x01, 'P', 0x04};
#endif

void setupScannerPins() {
    pinMode(PIN_QR_BCRES_CFG, OUTPUT);
    digitalWrite(PIN_QR_BCRES_CFG, HIGH);
#if defined(SCANNER_CONTROL_USE_TRIGGER)
    pinMode(PIN_QR_BCTRIG_CFG, OUTPUT);
    digitalWrite(PIN_QR_BCTRIG_CFG, HIGH);
#endif
    // Keep scanner reset and trigger lines inactive by default.
}

void initializeScanner() {
    if (scannerInitialized) {
        return;
    }
    scannerInitialized = true;
    pinMode(PIN_QR_BCRES_CFG, OUTPUT);
    digitalWrite(PIN_QR_BCRES_CFG, HIGH);
#if defined(SCANNER_CONTROL_USE_TRIGGER)
    pinMode(PIN_QR_BCTRIG_CFG, OUTPUT);
    digitalWrite(PIN_QR_BCTRIG_CFG, HIGH);
#endif
    delay(50);
    Serial.println("Scanner control pins configured.");
    Serial.println("Scanner initialization phase complete. Add actual N1-W setup commands if required.");
}

#if defined(SCANNER_CONTROL_USE_SERIAL)
void scannerOn(HardwareSerial& scannerSerial) {
    initializeScanner();
    scannerSerial.write(START_SCAN_CMD, sizeof(START_SCAN_CMD));
    Serial.println("Scanner ON via serial");
}

void scannerOff(HardwareSerial& scannerSerial) {
    initializeScanner();
    scannerSerial.write(STOP_SCAN_CMD, sizeof(STOP_SCAN_CMD));
    Serial.println("Scanner OFF via serial");
}
#else
void scannerOn() {
    initializeScanner();
    digitalWrite(PIN_QR_BCTRIG_CFG, LOW);
    Serial.println("Scanner ON");
}

void scannerOff() {
    initializeScanner();
    digitalWrite(PIN_QR_BCTRIG_CFG, HIGH);
    Serial.println("Scanner OFF");
}
#endif

void forwardScannerData(HardwareSerial& source, HardwareSerial& destination) {
    while (source.available()) {
        const char c = source.read();
        if (c == '\r' || c == '\n') {
            if (scannerBuffer.length() > 0) {
                destination.print(scannerBuffer);
                destination.print("\r\n");
                scannerBuffer = "";
            }
        } else {
            scannerBuffer += c;
            if (scannerBuffer.length() > 240) {
                destination.print(scannerBuffer);
                destination.print("\r\n");
                scannerBuffer = "";
            }
        }
    }
}
