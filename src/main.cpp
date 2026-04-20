#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Adafruit_NeoPixel.h>
#include "lv_conf.h"
#include <lvgl.h>
#include <vector>

#ifndef DISPLAY_UPDATE_INTERVAL_SEC
#define DISPLAY_UPDATE_INTERVAL_SEC 15
#endif

#ifndef PIN_BOOT_BUTTON
#define PIN_BOOT_BUTTON 4
#endif
#ifndef PIN_RGB_LED
#define PIN_RGB_LED 3
#endif
#ifndef PIN_EPD_BUSY_ACTIVE_LEVEL
#define EPD_BUSY_ACTIVE_LEVEL LOW
#endif

#ifndef PIN_EPD_CS
#define PIN_EPD_CS 36
#endif
#ifndef PIN_EPD_DC
#define PIN_EPD_DC 35
#endif
#ifndef PIN_EPD_RST
#define PIN_EPD_RST 34
#endif
#ifndef PIN_EPD_BUSY
#define PIN_EPD_BUSY 33
#endif
#ifndef PIN_SPI_SCK
#define PIN_SPI_SCK 37
#endif
#ifndef PIN_SPI_MOSI
#define PIN_SPI_MOSI 38
#endif
#ifndef PIN_SPI_MISO
#define PIN_SPI_MISO -1
#endif

#ifndef PIN_SCANNER_TX
#define PIN_SCANNER_TX 17
#endif
#ifndef PIN_SCANNER_RX
#define PIN_SCANNER_RX 18
#endif
#ifndef PIN_RX_ESP
#define PIN_RX_ESP 40
#endif
#ifndef PIN_TX_ESP
#define PIN_TX_ESP 39
#endif
#ifndef PIN_QR_BCRES
#define PIN_QR_BCRES 42
#endif
#ifndef PIN_QR_BCTRIG
#define PIN_QR_BCTRIG 45
#endif

constexpr uint16_t EPD_WIDTH = 200;
constexpr uint16_t EPD_HEIGHT = 200;
constexpr uint8_t EPD_ROTATION = 0;

constexpr int8_t PIN_EPD_CS_CFG = PIN_EPD_CS;
constexpr int8_t PIN_EPD_DC_CFG = PIN_EPD_DC;
constexpr int8_t PIN_EPD_RST_CFG = PIN_EPD_RST;
constexpr int8_t PIN_EPD_BUSY_CFG = PIN_EPD_BUSY;
constexpr int8_t PIN_SPI_SCK_CFG = PIN_SPI_SCK;
constexpr int8_t PIN_SPI_MOSI_CFG = PIN_SPI_MOSI;
constexpr int8_t PIN_SPI_MISO_CFG = PIN_SPI_MISO;
constexpr int8_t PIN_BOOT_BUTTON_CFG = PIN_BOOT_BUTTON;
constexpr int8_t PIN_RGB_LED_CFG = PIN_RGB_LED;
constexpr int8_t PIN_SCANNER_TX_CFG = PIN_SCANNER_TX;
constexpr int8_t PIN_SCANNER_RX_CFG = PIN_SCANNER_RX;
constexpr int8_t PIN_RX_ESP_CFG = PIN_RX_ESP;
constexpr int8_t PIN_TX_ESP_CFG = PIN_TX_ESP;
constexpr int8_t PIN_QR_BCRES_CFG = PIN_QR_BCRES;
constexpr int8_t PIN_QR_BCTRIG_CFG = PIN_QR_BCTRIG;

Adafruit_NeoPixel rgb_led(4, PIN_RGB_LED_CFG, NEO_GRB + NEO_KHZ800);
GxEPD2_BW<GxEPD2_154_GDEY0154D67, GxEPD2_154_GDEY0154D67::HEIGHT> epd_display(GxEPD2_154_GDEY0154D67(PIN_EPD_CS_CFG, PIN_EPD_DC_CFG, PIN_EPD_RST_CFG, PIN_EPD_BUSY_CFG));
HardwareSerial mhSerial(1);
HardwareSerial scannerSerial(2);

static lv_obj_t* display_label = nullptr;
static bool scannerInitialized = false;
static String scannerBuffer;

struct FontDefinition {
    const lv_font_t* font;
    uint8_t maxCharsPerLine;
    uint8_t maxLines;
    const char* name;
};

static const FontDefinition fontDefinitions[] = {
    {&lv_font_montserrat_48, 2, 2, "montserrat_48"},
    {&lv_font_montserrat_48, 3, 3, "montserrat_48"},
    {&lv_font_montserrat_28, 5, 4, "montserrat_28"},
    {&lv_font_montserrat_28, 10, 8, "montserrat_28"},
    {&lv_font_montserrat_14, 12, 12, "montserrat_14"},
    {&lv_font_montserrat_14, 18, 15, "montserrat_14"},
};

static const uint8_t kFontCount = sizeof(fontDefinitions) / sizeof(fontDefinitions[0]);

bool isNumericOnly(const String& text) {
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text.charAt(i);
        if (c == '\r' || c == '\n' || c == ' ') continue;
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

String normalizeText(const String& text) {
    String normalized;
    normalized.reserve(text.length());
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text.charAt(i);
        if (c == '\r') {
            normalized += '\n';
        } else if (c == '\t') {
            normalized += ' ';
        } else {
            normalized += c;
        }
    }
    return normalized;
}

std::vector<String> wrapText(const String& text, const FontDefinition& fontDef) {
    std::vector<String> lines;
    String currentLine;
    String token;
    for (size_t i = 0; i <= text.length(); ++i) {
        char c = i < text.length() ? text.charAt(i) : ' ';
        if (c == '\n') {
            if (token.length()) {
                if (currentLine.length()) {
                    if (currentLine.length() + 1 + token.length() <= fontDef.maxCharsPerLine) {
                        currentLine += ' ';
                        currentLine += token;
                    } else {
                        lines.push_back(currentLine);
                        currentLine = token;
                    }
                } else {
                    currentLine = token;
                }
                token = "";
            }
            lines.push_back(currentLine);
            currentLine = "";
        } else if (c == ' ' || i == text.length()) {
            if (token.length()) {
                if (currentLine.length()) {
                    if (currentLine.length() + 1 + token.length() <= fontDef.maxCharsPerLine) {
                        currentLine += ' ';
                        currentLine += token;
                    } else {
                        lines.push_back(currentLine);
                        currentLine = token;
                    }
                } else {
                    currentLine = token;
                }
                token = "";
            }
            if (i == text.length()) {
                break;
            }
        } else {
            token += c;
            if (token.length() > fontDef.maxCharsPerLine) {
                if (currentLine.length()) {
                    lines.push_back(currentLine);
                    currentLine = "";
                }
                while (token.length() > fontDef.maxCharsPerLine) {
                    lines.push_back(token.substring(0, fontDef.maxCharsPerLine));
                    token = token.substring(fontDef.maxCharsPerLine);
                }
                if (token.length()) {
                    currentLine = token;
                    token = "";
                }
            }
        }
        if (currentLine.length() > fontDef.maxCharsPerLine) {
            lines.push_back(currentLine.substring(0, fontDef.maxCharsPerLine));
            currentLine = currentLine.substring(fontDef.maxCharsPerLine);
        }
    }
    if (currentLine.length() || lines.empty()) {
        lines.push_back(currentLine);
    }
    return lines;
}

String joinLines(const std::vector<String>& lines) {
    String result;
    for (size_t i = 0; i < lines.size(); ++i) {
        result += lines[i];
        if (i + 1 < lines.size()) {
            result += '\n';
        }
    }
    return result;
}

bool fitsInFont(const String& text, const FontDefinition& fontDef) {
    auto lines = wrapText(text, fontDef);
    return lines.size() <= fontDef.maxLines;
}

String truncateText(const String& text, const FontDefinition& fontDef) {
    auto lines = wrapText(text, fontDef);
    if (lines.size() <= fontDef.maxLines) {
        return joinLines(lines);
    }
    lines.resize(fontDef.maxLines);
    String& last = lines.back();
    if (last.length() > 4) {
        last = last.substring(0, last.length() - 4);
        last += " ...";
    } else {
        last = " ...";
    }
    return joinLines(lines);
}

uint8_t selectFontIndex(const String& text) {
    const String normalized = normalizeText(text);
    const bool numeric = isNumericOnly(normalized);
    std::vector<uint8_t> candidates;
    if (numeric) {
        candidates = {0, 1, 2, 3, 4, 5, 6, 7};
    } else {
        candidates = {3, 4, 5, 6, 7};
    }
    for (uint8_t index : candidates) {
        if (fitsInFont(normalized, fontDefinitions[index])) {
            return index;
        }
    }
    return candidates.back();
}

void clearDisplay() {
    epd_display.setFullWindow();
    epd_display.fillScreen(GxEPD_WHITE);
    epd_display.display(false);
    if (display_label) {
        lv_label_set_text(display_label, "");
        lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
    }
}

void initializeScanner() {
    if (scannerInitialized) {
        return;
    }
    scannerInitialized = true;
    pinMode(PIN_QR_BCRES_CFG, OUTPUT);
    pinMode(PIN_QR_BCTRIG_CFG, OUTPUT);
    digitalWrite(PIN_QR_BCRES_CFG, HIGH);
    digitalWrite(PIN_QR_BCTRIG_CFG, LOW);
    delay(50);
    Serial.println("Scanner control pins configured.");
    Serial.println("Scanner initialization phase complete. Add actual N1-W setup commands if required.");
}

void displayText(const String& raw_text) {
    const String normalized = normalizeText(raw_text);
    const uint8_t fontIndex = selectFontIndex(normalized);
    const auto& fontDef = fontDefinitions[fontIndex];
    String output = fitsInFont(normalized, fontDef) ? joinLines(wrapText(normalized, fontDef)) : truncateText(normalized, fontDef);

    lv_obj_set_style_text_font(display_label, fontDef.font, LV_PART_MAIN);
    lv_label_set_text(display_label, output.c_str());
    lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
    Serial.printf("Display text using %s\n", fontDef.name);
}

void applyLedValues(const uint8_t* values) {
    for (uint8_t i = 0; i < 4; ++i) {
        const uint8_t r = values[i * 3];
        const uint8_t g = values[i * 3 + 1];
        const uint8_t b = values[i * 3 + 2];
        rgb_led.setPixelColor(i, rgb_led.Color(r, g, b));
    }
    rgb_led.show();
}

void forwardScannerData() {
    while (scannerSerial.available()) {
        const char c = scannerSerial.read();
        if (c == '\r' || c == '\n') {
            if (scannerBuffer.length() > 0) {
                mhSerial.print(scannerBuffer);
                mhSerial.print("\r\n");
                scannerBuffer = "";
            }
        } else {
            scannerBuffer += c;
            if (scannerBuffer.length() > 240) {
                mhSerial.print(scannerBuffer);
                mhSerial.print("\r\n");
                scannerBuffer = "";
            }
        }
    }
}

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

CommandParser parser;

void processMhSerialByte(uint8_t b) {
    switch (parser.state) {
        case PacketState::WAIT_COMMAND:
            if (b == 0x00 || b == 0x01 || b == 0x02) {
                parser.command = b;
                parser.state = (b == 0x00) ? PacketState::WAIT_VALUE : PacketState::WAIT_VALUE;
            }
            break;
        case PacketState::WAIT_VALUE:
            if (parser.command == 0x00) {
                if (b == 0x00) {
                    Serial.println("Command 0x00 0x00: clear full display");
                    clearDisplay();
                } else if (b == 0x01) {
                    Serial.println("Command 0x00 0x01: scanner init + clear display");
                    initializeScanner();
                    clearDisplay();
                } else {
                    Serial.printf("Unknown 0x00 command: 0x%02X\n", b);
                }
                parser.state = PacketState::WAIT_COMMAND;
            } else if (parser.command == 0x01) {
                parser.payloadLen = b;
                parser.payloadIndex = 0;
                if (parser.payloadLen == 0) {
                    displayText("");
                    parser.state = PacketState::WAIT_COMMAND;
                } else {
                    parser.state = PacketState::RECEIVE_PAYLOAD;
                }
            } else if (parser.command == 0x02) {
                parser.payloadLen = 12;
                parser.payloadIndex = 0;
                parser.state = PacketState::RECEIVE_PAYLOAD;
            }
            break;
        case PacketState::RECEIVE_PAYLOAD:
            parser.payload[parser.payloadIndex++] = b;
            if (parser.payloadIndex >= parser.payloadLen) {
                if (parser.command == 0x01) {
                    parser.payload[parser.payloadLen] = '\0';
                    displayText(String(reinterpret_cast<char*>(parser.payload)));
                } else if (parser.command == 0x02) {
                    applyLedValues(parser.payload);
                }
                parser.state = PacketState::WAIT_COMMAND;
            }
            break;
    }
}

void handleMhSerial() {
    while (mhSerial.available()) {
        const uint8_t b = mhSerial.read();
        processMhSerialByte(b);
    }
}

void setupDisplay() {
    pinMode(PIN_EPD_BUSY_CFG, INPUT_PULLUP);
    pinMode(PIN_EPD_RST_CFG, OUTPUT);
    digitalWrite(PIN_EPD_RST_CFG, HIGH);
    delay(10);
    digitalWrite(PIN_EPD_RST_CFG, LOW);
    delay(10);
    digitalWrite(PIN_EPD_RST_CFG, HIGH);
    delay(10);

    SPI.begin(PIN_SPI_SCK_CFG, PIN_SPI_MISO_CFG, PIN_SPI_MOSI_CFG, PIN_EPD_CS_CFG);
    epd_display.init(115200);
    epd_display.epd2.setBusyCallback([](const void*) {});
    epd_display.setRotation(EPD_ROTATION);
    epd_display.setFullWindow();
    epd_display.fillScreen(GxEPD_WHITE);
    epd_display.display(false);
}

void setupRgbLed() {
    rgb_led.begin();
    rgb_led.clear();
    rgb_led.show();
}

void setupScannerPins() {
    pinMode(PIN_QR_BCRES_CFG, OUTPUT);
    pinMode(PIN_QR_BCTRIG_CFG, OUTPUT);
    digitalWrite(PIN_QR_BCRES_CFG, LOW);
    digitalWrite(PIN_QR_BCTRIG_CFG, LOW);
}

static void epd_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    lv_area_t flush_area = *area;
    if (flush_area.x1 < 0) flush_area.x1 = 0;
    if (flush_area.y1 < 0) flush_area.y1 = 0;
    if (flush_area.x2 >= static_cast<int32_t>(EPD_WIDTH)) flush_area.x2 = EPD_WIDTH - 1;
    if (flush_area.y2 >= static_cast<int32_t>(EPD_HEIGHT)) flush_area.y2 = EPD_HEIGHT - 1;
    const uint16_t width = static_cast<uint16_t>(flush_area.x2 - flush_area.x1 + 1);
    const uint16_t height = static_cast<uint16_t>(flush_area.y2 - flush_area.y1 + 1);
    epd_display.setPartialWindow(flush_area.x1, flush_area.y1, width, height);
    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            const uint32_t index = static_cast<uint32_t>(y) * width + x;
            const lv_color_t color = color_p[index];
            const uint16_t epdColor = (color.full == lv_color_white().full) ? GxEPD_WHITE : GxEPD_BLACK;
            epd_display.drawPixel(flush_area.x1 + x, flush_area.y1 + y, epdColor);
        }
    }
    epd_display.display(false);
    lv_disp_flush_ready(drv);
}

void setupLvgl() {
    lv_init();
    static lv_color_t lv_framebuffer[EPD_WIDTH * 40];
    static lv_disp_draw_buf_t draw_buf;
    static lv_disp_drv_t disp_drv;
    lv_disp_draw_buf_init(&draw_buf, lv_framebuffer, nullptr, EPD_WIDTH * 40);
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EPD_WIDTH;
    disp_drv.ver_res = EPD_HEIGHT;
    disp_drv.flush_cb = epd_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 0;
    lv_disp_drv_register(&disp_drv);
}

void buildUi() {
    lv_obj_t* screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    display_label = lv_label_create(screen);
    lv_obj_set_width(display_label, EPD_WIDTH);
    lv_label_set_long_mode(display_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(display_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(display_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_bg_color(display_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(display_label, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_text(display_label, "Ready");
    lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
}

void setup() {
    Serial.begin(115200);
    mhSerial.begin(115200, SERIAL_8N1, PIN_RX_ESP_CFG, PIN_TX_ESP_CFG);
    scannerSerial.begin(115200, SERIAL_8N1, PIN_SCANNER_RX_CFG, PIN_SCANNER_TX_CFG);
    pinMode(PIN_BOOT_BUTTON_CFG, INPUT_PULLUP);
    setupRgbLed();
    setupScannerPins();
    setupDisplay();
    setupLvgl();
    buildUi();
    Serial.println("EPaperQr firmware started");
}

void loop() {
    handleMhSerial();
    forwardScannerData();
    lv_timer_handler();
    delay(5);
}
