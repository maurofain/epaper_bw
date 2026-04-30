#include "master_protocol.h"
#include "display.h"
#include "led_control.h"
#include "scanner_control.h"

#include <driver/uart.h>
#include <esp_log.h>
#include <esp_system.h>
#include <string>
#if defined(MASTER_PROTOCOL_USE_USB_CONSOLE)
#include <unistd.h>
#include <errno.h>
#endif

static const char* TAG = "MasterProtocol";

constexpr uint8_t PKT_COMMAND = 0x00;
constexpr uint8_t PKT_TEXT = 0x01;
constexpr uint8_t PKT_LED_VALUES = 0x02;

constexpr uint8_t CMD_DISPLAY_REFRESH = 0x00;
constexpr uint8_t CMD_SCANNER_INIT_REFRESH = 0x01;
constexpr uint8_t CMD_SCANNER_ON = 0x02;
constexpr uint8_t CMD_SCANNER_OFF = 0x03;
constexpr uint8_t CMD_THEME_NORMAL = 0x04;
constexpr uint8_t CMD_THEME_INVERT = 0x05;
constexpr uint8_t CMD_MASTER_INIT = 0xAA;
constexpr uint8_t CMD_REBOOT = 0xBB;
constexpr uint8_t CMD_SHOW_LOGO = 0xFF;

enum class PacketState {
    WAIT_COMMAND,
    WAIT_VALUE,
    RECEIVE_PAYLOAD,
};

// Strips the § prefix (0xA7 Latin-1 or 0xC2+0xA7 UTF-8) from text.
// Returns true if the prefix was found and stripped (caller must clear display first).
static bool stripSectionSign(std::string &text) {
    if (text.size() >= 2 && static_cast<uint8_t>(text[0]) == 0xC2 && static_cast<uint8_t>(text[1]) == 0xA7) {
        text.erase(0, 2);
        return true;
    }
    if (!text.empty() && static_cast<uint8_t>(text[0]) == 0xA7) {
        text.erase(0, 1);
        return true;
    }
    return false;
}

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
static bool gMasterInitialized = false;

static void processMhSerialByte(uint8_t b
#if defined(SCANNER_CONTROL_USE_SERIAL)
    , uart_port_t scannerPort
#endif
) {
    if (!gMasterInitialized) {
        if (parser.state == PacketState::WAIT_COMMAND) {
            if (b == PKT_COMMAND) {
                parser.command = b;
                parser.state = PacketState::WAIT_VALUE;
            }
            return;
        }
        if (parser.state == PacketState::WAIT_VALUE) {
            if (parser.command == PKT_COMMAND && b == CMD_MASTER_INIT) {
                gMasterInitialized = true;
                ESP_LOGI(TAG, "Command 0x00 0xAA: master init received");
            } else if (parser.command == PKT_COMMAND && b == CMD_REBOOT) {
                ESP_LOGI(TAG, "Command 0x00 0xBB: reboot esp32s2 (pre-init)");
                esp_restart();
            } else {
                ESP_LOGW(TAG, "Master init packet ignored: 0x%02X", b);
            }
            parser.state = PacketState::WAIT_COMMAND;
            return;
        }
    }
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
                ESP_LOGI(TAG, "Master serial command received: 0x00 0x%02X", b);
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
                } else if (b == CMD_THEME_NORMAL) {
                    ESP_LOGI(TAG, "Command 0x00 0x04: theme normal (white bg/black text)");
                    setDisplayTheme(false);
                } else if (b == CMD_THEME_INVERT) {
                    ESP_LOGI(TAG, "Command 0x00 0x05: theme inverted (black bg/white text)");
                    setDisplayTheme(true);
                } else if (b == CMD_MASTER_INIT) {
                    ESP_LOGI(TAG, "Command 0x00 0xAA: master init received");
                    gMasterInitialized = true;
                } else if (b == CMD_REBOOT) {
                    ESP_LOGI(TAG, "Command 0x00 0xBB: reboot esp32s2");
                    esp_restart();
                } else if (b == CMD_SHOW_LOGO) {
                    ESP_LOGI(TAG, "Command 0x00 0xFF: display logo");
                    displayLogo();
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
                        if (stripSectionSign(text)) {
                            ESP_LOGI(TAG, "§ prefix: clearing display before extended text");
                            clearDisplay();
                        }
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
                        // Log hex bytes to diagnose encoding/baud issues
                        char hexbuf[parser.payloadLen * 3 + 1];
                        for (size_t i = 0; i < parser.payloadLen; i++) {
                            snprintf(hexbuf + i*3, 4, "%02X ", parser.payload[i]);
                        }
                        hexbuf[parser.payloadLen * 3] = '\0';
                        ESP_LOGI(TAG, "Complete text payload: %s [hex: %s]", text.c_str(), hexbuf);
                        if (stripSectionSign(text)) {
                            ESP_LOGI(TAG, "§ prefix: clearing display before text");
                            clearDisplay();
                        }
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
#if defined(MASTER_PROTOCOL_USE_USB_CONSOLE)
    while (true) {
        uint8_t b;
        int len = read(STDIN_FILENO, &b, 1);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            ESP_LOGW(TAG, "USB console read failed: err=%d", errno);
            break;
        }
        if (len == 0) {
            break;
        }
        processMhSerialByte(b
#if defined(SCANNER_CONTROL_USE_SERIAL)
            , scannerPort
#endif
        );
    }
#else
    size_t bufferedLen = 0;
    esp_err_t err = uart_get_buffered_data_len(source, &bufferedLen);
    if (err != ESP_OK) {
        static bool uart_error_reported = false;
        if (!uart_error_reported) {
            ESP_LOGW(TAG, "UART master handle skipped: uart_get_buffered_data_len failed (err=0x%02X)", err);
            uart_error_reported = true;
        }
        return;
    }
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
#endif
}
