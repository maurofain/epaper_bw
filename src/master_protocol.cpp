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
    bool extendedText = false;
    uint8_t fontNumber = 0;
    uint8_t posX = 0;
    uint8_t posY = 0;
    uint8_t payload[256] = {0};
};

static CommandParser parser;

static void processMhSerialByte(uint8_t b
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , HardwareSerial& scannerSerial
#endif
) {
    Serial.printf("[DEBUG] Received byte: 0x%02X state=%d\n", b, static_cast<int>(parser.state));
    switch (parser.state) {
        case PacketState::WAIT_COMMAND:
            if (b == PKT_COMMAND || b == PKT_TEXT || b == PKT_LED_VALUES) {
                parser.command = b;
                parser.state = PacketState::WAIT_VALUE;
                Serial.printf("[DEBUG] Packet type %u detected\n", b);
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
                parser.payloadIndex = 0;
                parser.extendedText = false;
                parser.fontNumber = 0;
                parser.posX = 0;
                parser.posY = 0;
                if (b == 0xFF) {
                    parser.extendedText = true;
                    parser.payloadLen = 0;
                    parser.state = PacketState::RECEIVE_PAYLOAD;
                } else {
                    parser.payloadLen = b;
                    if (parser.payloadLen == 0) {
                        displayText("");
                        parser.state = PacketState::WAIT_COMMAND;
                    } else {
                        parser.state = PacketState::RECEIVE_PAYLOAD;
                    }
                }
            } else if (parser.command == PKT_LED_VALUES) {
                parser.payloadLen = 12;
                parser.payloadIndex = 0;
                parser.state = PacketState::RECEIVE_PAYLOAD;
            }
            break;
        case PacketState::RECEIVE_PAYLOAD:
            if (parser.extendedText) {
                if (parser.payloadIndex == 0) {
                    parser.fontNumber = b;
                } else if (parser.payloadIndex == 1) {
                    parser.posX = b;
                } else if (parser.payloadIndex == 2) {
                    parser.posY = b;
                } else {
                    if (b == 0x00) {
                        parser.payload[parser.payloadLen] = '\0';
                        Serial.printf("[DEBUG] Complete extended text payload: font=%u x=%u y=%u text=%s\n",
                                      parser.fontNumber, parser.posX, parser.posY,
                                      reinterpret_cast<char*>(parser.payload));
                        displayText(String(reinterpret_cast<char*>(parser.payload)), parser.fontNumber, parser.posX, parser.posY);
                        parser.state = PacketState::WAIT_COMMAND;
                        break;
                    }
                    if (parser.payloadLen < sizeof(parser.payload) - 1) {
                        parser.payload[parser.payloadLen++] = b;
                    }
                }
                parser.payloadIndex++;
            } else {
                parser.payload[parser.payloadIndex++] = b;
                if (parser.payloadIndex >= parser.payloadLen) {
                    if (parser.command == PKT_TEXT) {
                        parser.payload[parser.payloadLen] = '\0';
                        Serial.printf("[DEBUG] Complete text payload: %s\n", reinterpret_cast<char*>(parser.payload));
                        displayText(String(reinterpret_cast<char*>(parser.payload)));
                    } else if (parser.command == PKT_LED_VALUES) {
                        Serial.println("[DEBUG] Complete LED payload received");
                        applyLedValues(parser.payload);
                    }
                    parser.state = PacketState::WAIT_COMMAND;
                }
            }
            break;
    }
}

void handleMasterSerial(HardwareSerial& source
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , HardwareSerial& scannerSerial
#endif
) {
    if (!source.available()) {
        return;
    }
    Serial.printf("[DEBUG] Master serial available: %u bytes\n", source.available());
    while (source.available()) {
        const uint8_t b = source.read();
        processMhSerialByte(b
#if defined(SCANNER_CONTROL_USE_SERIAL)
            , scannerSerial
#endif
        );
    }
}
