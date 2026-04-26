#include "master_protocol.h"
#include "display.h"
#include "led_control.h"
#include "scanner_control.h"

#include <driver/uart.h>
#include <esp_log.h>
#include <string>

static const char* TAG = "MasterProtocol";

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
    , uart_port_t scannerPort
#endif
) {
    ESP_LOGD(TAG, "Received byte: 0x%02X state=%d", b, static_cast<int>(parser.state));
    switch (parser.state) {
        case PacketState::WAIT_COMMAND:
            if (b == PKT_COMMAND || b == PKT_TEXT || b == PKT_LED_VALUES) {
                parser.command = b;
                parser.state = PacketState::WAIT_VALUE;
                ESP_LOGD(TAG, "Packet type %u detected", b);
            }
            break;
        case PacketState::WAIT_VALUE:
            if (parser.command == PKT_COMMAND) {
                if (b == CMD_DISPLAY_REFRESH) {
                    ESP_LOGI(TAG, "Command 0x00 0x00: clear full display");
                    clearDisplay();
                } else if (b == CMD_SCANNER_INIT_REFRESH) {
                    ESP_LOGI(TAG, "Command 0x00 0x01: scanner init + clear display");
                    initializeScanner();
                    clearDisplay();
                } else if (b == CMD_SCANNER_ON) {
                    ESP_LOGI(TAG, "Command 0x00 0x02: scanner ON");
#if defined(SCANNER_CONTROL_USE_SERIAL)
                    scannerOn(scannerPort);
#else
                    scannerOn();
#endif
                } else if (b == CMD_SCANNER_OFF) {
                    ESP_LOGI(TAG, "Command 0x00 0x03: scanner OFF");
#if defined(SCANNER_CONTROL_USE_SERIAL)
                    scannerOff(scannerPort);
#else
                    scannerOff();
#endif
                } else if (b == CMD_LED_OFF) {
                    ESP_LOGI(TAG, "Command 0x00 0x04: LED OFF");
                    ledOff();
                } else {
                    ESP_LOGW(TAG, "Unknown 0x00 command: 0x%02X", b);
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
                        displayText(std::string());
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
                        std::string text(reinterpret_cast<char*>(parser.payload), parser.payloadLen);
                        ESP_LOGI(TAG, "Complete extended text payload: font=%u x=%u y=%u text=%s",
                                 parser.fontNumber, parser.posX, parser.posY, text.c_str());
                        displayText(text, parser.fontNumber, parser.posX, parser.posY);
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
                        std::string text(reinterpret_cast<char*>(parser.payload), parser.payloadLen);
                        ESP_LOGI(TAG, "Complete text payload: %s", text.c_str());
                        displayText(text);
                    } else if (parser.command == PKT_LED_VALUES) {
                        ESP_LOGI(TAG, "Complete LED payload received");
                        applyLedValues(parser.payload);
                    }
                    parser.state = PacketState::WAIT_COMMAND;
                }
            }
            break;
    }
}

void handleMasterSerial(uart_port_t source
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , uart_port_t scannerPort
#endif
) {
    size_t bufferedLen = 0;
    uart_get_buffered_data_len(source, &bufferedLen);
    if (bufferedLen == 0) {
        return;
    }
    ESP_LOGD(TAG, "Master serial available: %u bytes", static_cast<unsigned>(bufferedLen));
    while (true) {
        uint8_t b;
        int len = uart_read_bytes(source, &b, 1, 0);
        if (len <= 0) {
            break;
        }
        processMhSerialByte(b
#if defined(SCANNER_CONTROL_USE_SERIAL)
            , scannerPort
#endif
        );
    }
}
