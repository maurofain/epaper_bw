#include "master_protocol.h"
#include "display.h"
#include "led_control.h"
#include "scanner_control.h"
#include "conditional_log.h"
#include "GDEY0154D67.h"

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
constexpr uint8_t CMD_MASTER_DISCONNECT = 0xEE;
constexpr uint8_t CMD_REBOOT = 0xBB;
constexpr uint8_t CMD_SHOW_LOGO = 0xFF;

enum class PacketState {
    WAIT_COMMAND,
    WAIT_VALUE,
    RECEIVE_PAYLOAD,
};

// Special text flags for display control
struct TextSpecialFlags {
    bool hasPartialRefresh = false;  // § flag: partial refresh before display
    bool useBoldFont = false;        // ç flag: use bold font (9 = GoogleSansBold140)
    bool useMontserrat = false;      // £ flag: use Montserrat font family
};

// Strips special prefixes (§, ç, £) from text and returns their flags.
// § (0xA7 Latin-1 or 0xC2+0xA7 UTF-8) = partial refresh
// ç (0xE7 Latin-1 or 0xC3+0xA7 UTF-8) = use bold font
// £ (0xA3 Latin-1 or 0xC2+0xA3 UTF-8) = use Montserrat font family
// Recursively checks for multiple special prefixes; order doesn't matter.
static TextSpecialFlags stripSpecialPrefixes(std::string &text) {
    TextSpecialFlags flags;
    
    // Check for § (0xC2+0xA7 UTF-8 or 0xA7 Latin-1)
    if (text.size() >= 2 && static_cast<uint8_t>(text[0]) == 0xC2 && static_cast<uint8_t>(text[1]) == 0xA7) {
        text.erase(0, 2);
        flags.hasPartialRefresh = true;
    } else if (!text.empty() && static_cast<uint8_t>(text[0]) == 0xA7) {
        text.erase(0, 1);
        flags.hasPartialRefresh = true;
    }
    
    // Check for ç (0xC3+0xA7 UTF-8 or 0xE7 Latin-1)
    if (text.size() >= 2 && static_cast<uint8_t>(text[0]) == 0xC3 && static_cast<uint8_t>(text[1]) == 0xA7) {
        text.erase(0, 2);
        flags.useBoldFont = true;
    } else if (!text.empty() && static_cast<uint8_t>(text[0]) == 0xE7) {
        text.erase(0, 1);
        flags.useBoldFont = true;
    }
    
    // Check for £ (0xC2+0xA3 UTF-8 or 0xA3 Latin-1)
    if (text.size() >= 2 && static_cast<uint8_t>(text[0]) == 0xC2 && static_cast<uint8_t>(text[1]) == 0xA3) {
        text.erase(0, 2);
        flags.useMontserrat = true;
    } else if (!text.empty() && static_cast<uint8_t>(text[0]) == 0xA3) {
        text.erase(0, 1);
        flags.useMontserrat = true;
    }
    
    // Recursively check for additional special prefixes
    if (!text.empty()) {
        uint8_t firstByte = static_cast<uint8_t>(text[0]);
        // Check if next character might be special (starts with 0xC or 0xE for UTF-8, or 0xA3/0xA7/0xE7 for Latin-1)
        if (firstByte == 0xC2 || firstByte == 0xC3 || firstByte == 0xA3 || firstByte == 0xA7 || firstByte == 0xE7) {
            TextSpecialFlags moreFlags = stripSpecialPrefixes(text);
            flags.hasPartialRefresh |= moreFlags.hasPartialRefresh;
            flags.useBoldFont |= moreFlags.useBoldFont;
            flags.useMontserrat |= moreFlags.useMontserrat;
        }
    }
    
    return flags;
}

// Strips the § prefix (0xA7 Latin-1 or 0xC2+0xA7 UTF-8) from text.
// Returns true if the prefix was found and stripped (caller must clear display first).
// DEPRECATED: use stripSpecialPrefixes() instead
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
                _LOGI("Command 0x00 0xAA: master init received");
            } else if (parser.command == PKT_COMMAND && b == CMD_REBOOT) {
                _LOGI("Command 0x00 0xBB: reboot esp32s2 (pre-init)");
                esp_restart();
            } else {
                _LOGW("Master init packet ignored: 0x%02X", b);
            }
            parser.state = PacketState::WAIT_COMMAND;
            return;
        }
    }
    _LOGD("Received byte: 0x%02X state=%d", b, static_cast<int>(parser.state));
    switch (parser.state) {
        case PacketState::WAIT_COMMAND:
            if (b == PKT_COMMAND || b == PKT_TEXT || b == PKT_LED_VALUES) {
                parser.command = b;
                parser.state = PacketState::WAIT_VALUE;
                _LOGD("Packet type %u detected", b);
            }
            break;
        case PacketState::WAIT_VALUE:
            if (parser.command == PKT_COMMAND) {
                _LOGI("Master serial command received: 0x00 0x%02X", b);
                if (b == CMD_DISPLAY_REFRESH) {
                    _LOGI("Command 0x00 0x00: clear display (partial refresh)");
                    clearDisplay();
                } else if (b == CMD_SCANNER_INIT_REFRESH) {
                    _LOGI("Command 0x00 0x01: scanner init + clear display");
                    initializeScanner();
                    clearDisplay();
                } else if (b == CMD_SCANNER_ON) {
                    _LOGI("Command 0x00 0x02: scanner ON");
#if defined(SCANNER_CONTROL_USE_SERIAL)
                    scannerOn(scannerPort);
#else
                    scannerOn();
#endif
                } else if (b == CMD_SCANNER_OFF) {
                    _LOGI("Command 0x00 0x03: scanner OFF");
#if defined(SCANNER_CONTROL_USE_SERIAL)
                    scannerOff(scannerPort);
#else
                    scannerOff();
#endif
                } else if (b == CMD_THEME_NORMAL) {
                    _LOGI("Command 0x00 0x04: theme normal (white bg/black text)");
                    setDisplayTheme(false);
                } else if (b == CMD_THEME_INVERT) {
                    _LOGI("Command 0x00 0x05: theme inverted (black bg/white text)");
                    setDisplayTheme(true);
                } else if (b == CMD_MASTER_INIT) {
                    _LOGI("Command 0x00 0xAA: master init received");
                    gMasterInitialized = true;
                } else if (b == CMD_MASTER_DISCONNECT) {
                    _LOGI("Command 0x00 0xEE: master disconnect - ignoring all input until new 0x00 0xAA");
                    gMasterInitialized = false;
                } else if (b == CMD_REBOOT) {
                    _LOGI("Command 0x00 0xBB: reboot esp32s2");
                    esp_restart();
                } else if (b == CMD_SHOW_LOGO) {
                    _LOGI("Command 0x00 0xFF: display logo");
                    displayLogo();
                } else {
                    _LOGW("Unknown 0x00 command: 0x%02X", b);
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
                        _LOGI("Complete extended text payload: font=%u x=%u y=%u text=%s",
                                 parser.fontNumber, parser.posX, parser.posY, text.c_str());
                        
                        TextSpecialFlags flags = stripSpecialPrefixes(text);
                        if (flags.hasPartialRefresh) {
                            _LOGI("§ prefix: LVGL buffer clear + partial refresh (optimized for fast updates)");
#if ENABLE_DISPLAY_LVGL
                            clearActiveScreen();  // Clear LVGL buffer only (no full e-paper refresh)
#endif
                            GDEY0154D67_refresh_partial();  // Fast partial refresh
                        }
                        if (flags.useBoldFont) {
                            _LOGI("ç flag: using bold font 9 instead of %u", parser.fontNumber);
                            parser.fontNumber = 9;  // Use GoogleSansBold140
                        }
                        
                        // Use clean display if § was present (smooth transitions with opaque background)
                        if (flags.hasPartialRefresh) {
                            displayTextClean(text, parser.fontNumber, parser.posX, parser.posY);
                        } else {
                            displayText(text, parser.fontNumber, parser.posX, parser.posY);
                        }
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
                        _LOGI("Complete text payload: %s [hex: %s]", text.c_str(), hexbuf);
                        
                        TextSpecialFlags flags = stripSpecialPrefixes(text);
                        if (flags.hasPartialRefresh) {
                            _LOGI("§ prefix: LVGL buffer clear + partial refresh (optimized for fast updates)");
#if ENABLE_DISPLAY_LVGL
                            clearActiveScreen();  // Clear LVGL buffer only (no full e-paper refresh)
#endif
                            GDEY0154D67_refresh_partial();  // Fast partial refresh
                        }
                        
                        // If bold font requested in auto mode, use extended format with font 9 at center
                        if (flags.useBoldFont) {
                            _LOGI("ç flag: using bold font 9 (auto mode text)");
                            displayText(text, 9, 100, 100);  // Font 9 = GoogleSansBold140, center position
                        } else {
                            displayText(text);  // Auto font selection
                        }
                    } else if (parser.command == PKT_LED_VALUES) {
                        _LOGI("Complete LED payload received");
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
            _LOGW("USB console read failed: err=%d", errno);
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
            _LOGW("UART master handle skipped: uart_get_buffered_data_len failed (err=0x%02X)", err);
            uart_error_reported = true;
        }
        return;
    }
    if (bufferedLen == 0) {
        return;
    }
    _LOGD("Master serial available: %u bytes", static_cast<unsigned>(bufferedLen));
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
