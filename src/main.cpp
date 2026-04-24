#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include "pin_config.h"
#include "display.h"
#include "led_control.h"
#include "scanner_control.h"
#include "master_protocol.h"
#include "lv_conf.h"
#include <lvgl.h>
#include <vector>

#ifndef DISPLAY_UPDATE_INTERVAL_SEC
#define DISPLAY_UPDATE_INTERVAL_SEC 15
#endif

constexpr uint16_t EPD_WIDTH = 200;
constexpr uint16_t EPD_HEIGHT = 200;
constexpr uint8_t EPD_ROTATION = 0;

GxEPD2_BW<GxEPD2_154_GDEY0154D67, GxEPD2_154_GDEY0154D67::HEIGHT> epd_display(GxEPD2_154_GDEY0154D67(PIN_EPD_CS_CFG, PIN_EPD_DC_CFG, PIN_EPD_RST_CFG, PIN_EPD_BUSY_CFG));
HardwareSerial mhSerial(1);
HardwareSerial scannerSerial(2);

// -----------------------------------------------------------------------------
// Sezione 1: Logica generale con loop principale
// -----------------------------------------------------------------------------

/**
 * setup
 * Inizializza le periferiche principali e avvia le interfacce seriali.
 * - Serial: porta di debug a 115200 baud
 * - mhSerial: connessione verso la scheda Master a 9800 baud
 * - scannerSerial: connessione verso lo scanner N1-W a 9600 baud
 * Non restituisce valori.
 */
void setup() {
    Serial.begin(115200);
    mhSerial.begin(9800, SERIAL_8N1, PIN_RX_ESP_CFG, PIN_TX_ESP_CFG);
    scannerSerial.begin(9600, SERIAL_8N1, PIN_SCANNER_RX_CFG, PIN_SCANNER_TX_CFG);
    pinMode(PIN_BOOT_BUTTON_CFG, INPUT_PULLUP);
    setupRgbLed();
    setupScannerPins();
    setupDisplay();
    setupLvgl();
    buildUi();
    Serial.println("EPaperQr firmware started");
}

/**
 * loop
 * Ciclo principale dell’applicazione.
 * - processa i byte in arrivo dalla scheda Master
 * - inoltra i dati ricevuti dallo scanner verso la scheda Master
 * - aggiorna la libreria LVGL
 * Non restituisce valori.
 */
void loop() {
    handleMasterSerial(mhSerial
#if defined(SCANNER_CONTROL_USE_SERIAL)
        , scannerSerial
#endif
    );
    forwardScannerData(scannerSerial, mhSerial);
    lv_timer_handler();
    delay(5);
}

// -----------------------------------------------------------------------------
// Sezione 3: Gestione del display e LVGL
// -----------------------------------------------------------------------------

static lv_obj_t* display_label = nullptr;

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

/**
 * isNumericOnly
 * Verifica se una stringa contiene solo caratteri numerici, ignorando CR, LF e spazi.
 * @param text Stringa in ingresso.
 * @return true se la stringa è composta esclusivamente da cifre numeriche, false altrimenti.
 */
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

/**
 * normalizeText
 * Converte CR in LF e tab in spazio per una gestione uniforme del testo.
 * @param text Stringa di input.
 * @return Stringa normalizzata.
 */
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

/**
 * wrapText
 * Avvolge il testo in righe in base alla larghezza massima permessa dal font.
 * - considera i caratteri di fine riga '\n'
 * - tiene conto del limite massimo di caratteri per riga del font
 * - tronca le parole che eccedono la larghezza massima
 * @param text Testo normalizzato da avvolgere.
 * @param fontDef Definizione del font contenente maxCharsPerLine.
 * @return Vettore di righe formattate.
 */
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

/**
 * joinLines
 * Costruisce una singola stringa unendo le righe separate da '\n'.
 * @param lines Vettore di righe di testo.
 * @return Stringa concatenata con line feed.
 */
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

/**
 * fitsInFont
 * Verifica se una stringa formattata con un font specifico rientra nel numero massimo di righe disponibili.
 * @param text Testo normalizzato.
 * @param fontDef Definizione del font.
 * @return true se il testo rientra, false altrimenti.
 */
bool fitsInFont(const String& text, const FontDefinition& fontDef) {
    auto lines = wrapText(text, fontDef);
    return lines.size() <= fontDef.maxLines;
}

/**
 * truncateText
 * Tronca il testo se supera le righe disponibili e aggiunge " ..." all’ultima riga.
 * @param text Testo normalizzato.
 * @param fontDef Definizione del font.
 * @return Testo eventualmente troncato.
 */
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

/**
 * selectFontIndex
 * Seleziona l’indice del font più adatto in base al contenuto del testo e alla lunghezza.
 * @param text Testo originale.
 * @return Indice del font selezionato dall’array fontDefinitions.
 */
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

/**
 * clearDisplay
 * Cancella completamente il display e resetta l’etichetta LVGL se presente.
 * Non prende parametri e non restituisce valori.
 */
void clearDisplay() {
    epd_display.setFullWindow();
    epd_display.fillScreen(GxEPD_WHITE);
    epd_display.display(false);
    if (display_label) {
        lv_label_set_text(display_label, "");
        lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
    }
}

/**
 * displayText
 * Gestisce la normalizzazione, selezione del font e la visualizzazione del testo su LVGL.
 * @param raw_text Testo ricevuto in input.
 * Non restituisce valori.
 */
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

/**
 * setupDisplay
 * Inizializza il controller EPD, il bus SPI e l’hardware di reset del display.
 * Non prende parametri e non restituisce valori.
 */
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

/**
 * epd_flush_cb
 * Callback LVGL per inviare una porzione di framebuffer al display e aggiornare l’EPD.
 * @param drv Driver di display LVGL.
 * @param area Area del framebuffer da aggiornare.
 * @param color_p Buffer dei pixel.
 */
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

/**
 * setupLvgl
 * Inizializza la libreria LVGL e la configurazione del buffer di disegno.
 * Non prende parametri e non restituisce valori.
 */
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

/**
 * buildUi
 * Costruisce l’interfaccia LVGL di base con un'etichetta centrale per il testo.
 * Non prende parametri e non restituisce valori.
 */
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

