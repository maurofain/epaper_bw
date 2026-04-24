#include <Arduino.h>
#include "master_protocol.h"
#include "display.h"
#include "led_control.h"
#include "scanner_control.h"

constexpr uint8_t PKT_COMMAND = 0x00;
constexpr uint8_t PKT_TEXT = 0x01;
constexpr uint8_t PKT_LED_VALUES = 0x02;

constexpr uint8_t CMD_DISPLAY_REFRESH = 0x00;
constexpr uint8_t CMD_SCANNER_INIT_REFRESH = 0x01;
constexpr uint8_t CMD_SCANNER_ON = 0x02;
constexpr uint8_t CMD_SCANNER_OFF = 0x03;
constexpr uint8_t CMD_LED_OFF = 0x04;

enum class PacketState {
    WAIT_COMMAND,
    WAIT_VALUE,
    RECEIVE_PAYLOAD,
};

struct CommandParser {
    PacketState state = PacketState::WAIT_COMMAND;
    uint8_t command = 0;
    uint8_t payloadLen = 0;
    uint8_t payloadIndex = 0;
    uint8_t payload[256] = {0};
};

static CommandParser parser;

static void processMhSerialByte(uint8_t b
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , HardwareSerial& scannerSerial
#endif
) {
    switch (parser.state) {
        case PacketState::WAIT_COMMAND:
            if (b == PKT_COMMAND || b == PKT_TEXT || b == PKT_LED_VALUES) {
                parser.command = b;
                parser.state = PacketState::WAIT_VALUE;
            }
            break;
        case PacketState::WAIT_VALUE:
            if (parser.command == PKT_COMMAND) {
                if (b == CMD_DISPLAY_REFRESH) {
                    Serial.println("Command 0x00 0x00: clear full display");
                    clearDisplay();
                } else if (b == CMD_SCANNER_INIT_REFRESH) {
                    Serial.println("Command 0x00 0x01: scanner init + clear display");
                    initializeScanner();
                    clearDisplay();
                } else if (b == CMD_SCANNER_ON) {
                    Serial.println("Command 0x00 0x02: scanner ON");
#if defined(SCANNER_CONTROL_USE_SERIAL)
                    scannerOn(scannerSerial);
#else
                    scannerOn();
#endif
                } else if (b == CMD_SCANNER_OFF) {
                    Serial.println("Command 0x00 0x03: scanner OFF");
#if defined(SCANNER_CONTROL_USE_SERIAL)
                    scannerOff(scannerSerial);
#else
                    scannerOff();
#endif
                } else if (b == CMD_LED_OFF) {
                    Serial.println("Command 0x00 0x04: LED OFF");
                    ledOff();
                } else {
                    Serial.printf("Unknown 0x00 command: 0x%02X\n", b);
                }
                parser.state = PacketState::WAIT_COMMAND;
            } else if (parser.command == PKT_TEXT) {
                parser.payloadLen = b;
                parser.payloadIndex = 0;
                if (parser.payloadLen == 0) {
                    displayText("");
                    parser.state = PacketState::WAIT_COMMAND;
                } else {
                    parser.state = PacketState::RECEIVE_PAYLOAD;
                }
            } else if (parser.command == PKT_LED_VALUES) {
                parser.payloadLen = 12;
                parser.payloadIndex = 0;
                parser.state = PacketState::RECEIVE_PAYLOAD;
            }
            break;
        case PacketState::RECEIVE_PAYLOAD:
            parser.payload[parser.payloadIndex++] = b;
            if (parser.payloadIndex >= parser.payloadLen) {
                if (parser.command == PKT_TEXT) {
                    parser.payload[parser.payloadLen] = '\0';
                    displayText(String(reinterpret_cast<char*>(parser.payload)));
                } else if (parser.command == PKT_LED_VALUES) {
                    applyLedValues(parser.payload);
                }
                parser.state = PacketState::WAIT_COMMAND;
            }
            break;
    }
}

void handleMasterSerial(HardwareSerial& source
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , HardwareSerial& scannerSerial
#endif
) {
    while (source.available()) {
        const uint8_t b = source.read();
        processMhSerialByte(b
#if defined(SCANNER_CONTROL_USE_SERIAL)
            , scannerSerial
#endif
        );
    }
}
