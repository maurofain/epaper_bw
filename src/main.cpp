#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_3C.h>
#include <Adafruit_NeoPixel.h>


#if USE_LVGL
#include <lvgl.h>
#endif

#ifndef USE_USER_FONT_70
#define USE_USER_FONT_70 0
#endif

#if USE_LVGL && USE_USER_FONT_70
#include "ui/fonts/user_font_70.h"
#endif

namespace {
constexpr uint16_t EPD_WIDTH = 250;
constexpr uint16_t EPD_HEIGHT = 122;
constexpr uint8_t EPD_ROTATION = 1;

#ifndef DISPLAY_UPDATE_INTERVAL_SEC
#define DISPLAY_UPDATE_INTERVAL_SEC 15
#endif

#ifndef COUNTER_TEXT_RED
#define COUNTER_TEXT_RED 0
#endif

#ifndef PIN_BOOT_BUTTON
#define PIN_BOOT_BUTTON 0
#endif

#ifndef PIN_RGB_LED
#define PIN_RGB_LED 40
#endif

#ifndef EPD_BUSY_ACTIVE_LEVEL
#define EPD_BUSY_ACTIVE_LEVEL HIGH
#endif

// Imposta qui i pin in base al tuo cablaggio ESP32-S3 + e-paper.
// Puoi anche sovrascriverli da platformio.ini con -DPIN_EPD_CS=... ecc.
#ifndef PIN_EPD_CS
#define PIN_EPD_CS 10
#endif
#ifndef PIN_EPD_DC
#define PIN_EPD_DC 9
#endif
#ifndef PIN_EPD_RST
#define PIN_EPD_RST 8
#endif
#ifndef PIN_EPD_BUSY
#define PIN_EPD_BUSY 7
#endif
#ifndef PIN_SPI_SCK
#define PIN_SPI_SCK 12
#endif
#ifndef PIN_SPI_MOSI
#define PIN_SPI_MOSI 11
#endif
#ifndef PIN_SPI_MISO
#define PIN_SPI_MISO -1
#endif

constexpr int8_t PIN_EPD_CS_CFG = PIN_EPD_CS;
constexpr int8_t PIN_EPD_DC_CFG = PIN_EPD_DC;
constexpr int8_t PIN_EPD_RST_CFG = PIN_EPD_RST;
constexpr int8_t PIN_EPD_BUSY_CFG = PIN_EPD_BUSY;
constexpr int8_t PIN_SPI_SCK_CFG = PIN_SPI_SCK;
constexpr int8_t PIN_SPI_MOSI_CFG = PIN_SPI_MOSI;
constexpr int8_t PIN_SPI_MISO_CFG = PIN_SPI_MISO;
constexpr int8_t PIN_BOOT_BUTTON_CFG = PIN_BOOT_BUTTON;
constexpr int8_t PIN_RGB_LED_CFG = PIN_RGB_LED;

bool counter_text_red = (COUNTER_TEXT_RED != 0);
uint32_t current_display_second = 0;
bool rgb_led_is_yellow = false;

Adafruit_NeoPixel rgb_led(1, PIN_RGB_LED_CFG, NEO_GRB + NEO_KHZ800);

GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> epd_display(GxEPD2_213_Z98c(PIN_EPD_CS_CFG, PIN_EPD_DC_CFG, PIN_EPD_RST_CFG, PIN_EPD_BUSY_CFG));

void set_rgb_led_yellow(bool enabled) {
  if (rgb_led_is_yellow == enabled) {
    return;
  }

  rgb_led_is_yellow = enabled;
  if (enabled) {
    rgb_led.setPixelColor(0, rgb_led.Color(255, 180, 0));
  } else {
    rgb_led.setPixelColor(0, rgb_led.Color(0, 0, 0));
  }
  rgb_led.show();
}

bool is_epd_busy_active() {
  return digitalRead(PIN_EPD_BUSY_CFG) == EPD_BUSY_ACTIVE_LEVEL;
}

void epd_busy_callback(const void*) {
  set_rgb_led_yellow(true);
}

void sync_rgb_led_with_epd_busy() {
  set_rgb_led_yellow(is_epd_busy_active());
}

#if USE_LVGL
static lv_color_t lv_framebuffer[EPD_WIDTH * EPD_HEIGHT];
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_obj_t* seconds_label = nullptr;

void normalize_lv_area(lv_area_t& area) {
  if (area.x1 < 0) {
    area.x1 = 0;
  }
  if (area.y1 < 0) {
    area.y1 = 0;
  }
  if (area.x2 >= static_cast<int32_t>(EPD_WIDTH)) {
    area.x2 = static_cast<int32_t>(EPD_WIDTH) - 1;
  }
  if (area.y2 >= static_cast<int32_t>(EPD_HEIGHT)) {
    area.y2 = static_cast<int32_t>(EPD_HEIGHT) - 1;
  }

  if (area.x2 < area.x1) {
    area.x2 = area.x1;
  }
}

uint16_t map_lvgl_color_to_epd(lv_color_t c) {
  uint8_t r = c.ch.red;
  uint8_t g = c.ch.green;
  uint8_t b = c.ch.blue;

  if (r > 22 && g < 10 && b < 10) {
    return GxEPD_RED;
  }

  uint16_t luminance = static_cast<uint16_t>(r) * 30U + static_cast<uint16_t>(g) * 59U + static_cast<uint16_t>(b) * 11U;
  luminance /= 100U;

  return (luminance > 14U) ? GxEPD_WHITE : GxEPD_BLACK;
}

void epd_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
  lv_area_t flush_area = *area;
  normalize_lv_area(flush_area);

  const uint16_t width = static_cast<uint16_t>(flush_area.x2 - flush_area.x1 + 1);
  const uint16_t height = static_cast<uint16_t>(flush_area.y2 - flush_area.y1 + 1);

  epd_display.setPartialWindow(flush_area.x1, flush_area.y1, width, height);

  for (uint16_t y = 0; y < height; ++y) {
    for (uint16_t x = 0; x < width; ++x) {
      const uint32_t index = static_cast<uint32_t>(y) * width + x;
      epd_display.drawPixel(flush_area.x1 + x, flush_area.y1 + y, map_lvgl_color_to_epd(color_p[index]));
    }
  }

  epd_display.display(false);
  lv_disp_flush_ready(drv);
}
#endif

void setup_display() {
  pinMode(PIN_EPD_BUSY_CFG, INPUT_PULLUP);
  pinMode(PIN_EPD_RST_CFG, OUTPUT);

  Serial.printf("Pins: CS=%d DC=%d RST=%d BUSY=%d SCK=%d MOSI=%d MISO=%d\n", PIN_EPD_CS_CFG, PIN_EPD_DC_CFG, PIN_EPD_RST_CFG, PIN_EPD_BUSY_CFG,
                PIN_SPI_SCK_CFG, PIN_SPI_MOSI_CFG, PIN_SPI_MISO_CFG);
  Serial.printf("BUSY level before init: %d\n", digitalRead(PIN_EPD_BUSY_CFG));

  digitalWrite(PIN_EPD_RST_CFG, HIGH);
  delay(10);
  digitalWrite(PIN_EPD_RST_CFG, LOW);
  delay(10);
  digitalWrite(PIN_EPD_RST_CFG, HIGH);
  delay(10);

  SPI.begin(PIN_SPI_SCK_CFG, PIN_SPI_MISO_CFG, PIN_SPI_MOSI_CFG, PIN_EPD_CS_CFG);
  epd_display.init(115200);
  epd_display.epd2.setBusyCallback(epd_busy_callback);
  epd_display.setRotation(EPD_ROTATION);
  epd_display.setFullWindow();
  epd_display.fillScreen(GxEPD_WHITE);
  epd_display.display(false);
  Serial.printf("BUSY level after init: %d\n", digitalRead(PIN_EPD_BUSY_CFG));
  sync_rgb_led_with_epd_busy();
}

void setup_rgb_led() {
  rgb_led.begin();
  rgb_led.clear();
  rgb_led.show();
}

void log_graphics_backend() {
#if USE_LVGL
  Serial.println("Graphics backend: LVGL + GxEPD2");
#else
  Serial.println("Graphics backend: Adafruit GFX (via GxEPD2), no LVGL");
#endif

  Serial.println("Panel driver: GxEPD2_213_Z98c");
  Serial.printf("Panel caps: partial=%d fast_partial=%d\n", epd_display.epd2.hasPartialUpdate, epd_display.epd2.hasFastPartialUpdate);
  Serial.printf("Display refresh interval: %u s\n", static_cast<unsigned>(DISPLAY_UPDATE_INTERVAL_SEC));
  Serial.printf("Counter color red: %u\n", static_cast<unsigned>(counter_text_red ? 1 : 0));
  Serial.printf("Boot button pin: %d\n", PIN_BOOT_BUTTON_CFG);
  Serial.printf("RGB LED pin: %d\n", PIN_RGB_LED_CFG);
}

#if USE_LVGL
void setup_lvgl() {
  lv_init();

  lv_disp_draw_buf_init(&draw_buf, lv_framebuffer, nullptr, EPD_WIDTH * EPD_HEIGHT);

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = EPD_WIDTH;
  disp_drv.ver_res = EPD_HEIGHT;
  disp_drv.flush_cb = epd_flush_cb;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.full_refresh = 0;
  lv_disp_drv_register(&disp_drv);
  Serial.println("Buffer and display driver initialized.");
}

void build_ui() {
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);

  seconds_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(seconds_label, counter_text_red ? lv_color_hex(0xFF0000) : lv_color_black(), LV_PART_MAIN);

#if USE_USER_FONT_70
  lv_obj_set_style_text_font(seconds_label, &user_font_70, LV_PART_MAIN);
#else
  lv_obj_set_style_text_font(seconds_label, &lv_font_montserrat_48, LV_PART_MAIN);
#endif

  lv_label_set_text(seconds_label, "0");
  lv_obj_center(seconds_label);
}

void draw_counter_value(uint32_t seconds) {
  current_display_second = seconds % 60U;

  if (seconds_label == nullptr) {
    return;
  }

  lv_obj_set_style_text_color(seconds_label, counter_text_red ? lv_color_hex(0xFF0000) : lv_color_black(), LV_PART_MAIN);

  static char text[16];
  snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(current_display_second));
  lv_label_set_text(seconds_label, text);
  lv_obj_center(seconds_label);
}

void update_counter() {
  static uint32_t last_second_seen = UINT32_MAX;
  static uint32_t last_draw_second = UINT32_MAX;
  const uint32_t now_second = millis() / 1000U;
  if (now_second == last_second_seen) {
    return;
  }
  last_second_seen = now_second;

  if (last_draw_second != UINT32_MAX && (now_second - last_draw_second) < DISPLAY_UPDATE_INTERVAL_SEC) {
    return;
  }

  const uint32_t display_second = now_second % 60U;
  Serial.printf("Sec: %lu\n", static_cast<unsigned long>(display_second));
  last_draw_second = now_second;
  draw_counter_value(display_second);
}
#else
struct Rect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

Rect make_union_rect(const Rect& a, const Rect& b) {
  const int16_t left = min(a.x, b.x);
  const int16_t top = min(a.y, b.y);
  const int16_t right = max(static_cast<int16_t>(a.x + a.w), static_cast<int16_t>(b.x + b.w));
  const int16_t bottom = max(static_cast<int16_t>(a.y + a.h), static_cast<int16_t>(b.y + b.h));
  return Rect{left, top, static_cast<int16_t>(right - left), static_cast<int16_t>(bottom - top)};
}

void normalize_rect(Rect& rect) {
  if (rect.x < 0) {
    rect.w += rect.x;
    rect.x = 0;
  }
  if (rect.y < 0) {
    rect.h += rect.y;
    rect.y = 0;
  }

  if (rect.x + rect.w > static_cast<int16_t>(EPD_WIDTH)) {
    rect.w = static_cast<int16_t>(EPD_WIDTH) - rect.x;
  }
  if (rect.y + rect.h > static_cast<int16_t>(EPD_HEIGHT)) {
    rect.h = static_cast<int16_t>(EPD_HEIGHT) - rect.y;
  }

  if (rect.w < 1) {
    rect.w = 1;
  }
  if (rect.h < 1) {
    rect.h = 1;
  }

  const int16_t aligned_x = static_cast<int16_t>(rect.x & ~0x07);
  const int16_t grow = rect.x - aligned_x;
  rect.x = aligned_x;
  rect.w = static_cast<int16_t>(rect.w + grow);

  if (rect.x + rect.w > static_cast<int16_t>(EPD_WIDTH)) {
    rect.w = static_cast<int16_t>(EPD_WIDTH) - rect.x;
  }
}

void draw_counter_adafruit(uint32_t seconds) {
  static bool has_previous_rect = false;
  static Rect previous_rect{0, 0, 0, 0};

  static char text[16];
  snprintf(text, sizeof(text), "%lu", static_cast<unsigned long>(seconds));
  Serial.printf("Drawing text: %s\n", text);
  epd_display.setTextSize(4);
  epd_display.setTextColor(counter_text_red ? GxEPD_RED : GxEPD_BLACK);

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0;
  uint16_t h = 0;
  epd_display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  const int16_t cursor_x = static_cast<int16_t>((EPD_WIDTH - w) / 2) - x1;
  const int16_t cursor_y = static_cast<int16_t>((EPD_HEIGHT - h) / 2) - y1;

  Rect current_rect{static_cast<int16_t>(cursor_x + x1 - 2), static_cast<int16_t>(cursor_y + y1 - 2), static_cast<int16_t>(w + 4), static_cast<int16_t>(h + 4)};
  Rect refresh_rect = has_previous_rect ? make_union_rect(previous_rect, current_rect) : current_rect;
  normalize_rect(refresh_rect);

  Serial.printf("refresh_rect: x=%d y=%d w=%d h=%d\n", refresh_rect.x, refresh_rect.y, refresh_rect.w, refresh_rect.h);

  epd_display.setPartialWindow(refresh_rect.x, refresh_rect.y, refresh_rect.w, refresh_rect.h);

  epd_display.firstPage();
  do {
    epd_display.fillRect(refresh_rect.x, refresh_rect.y, refresh_rect.w, refresh_rect.h, GxEPD_WHITE);

    epd_display.setCursor(cursor_x, cursor_y);
    epd_display.print(text);
  } while (epd_display.nextPage());

  previous_rect = current_rect;
  has_previous_rect = true;
}

void draw_counter_value(uint32_t seconds) {
  current_display_second = seconds % 60U;
  draw_counter_adafruit(current_display_second);
}

void update_counter() {
  static uint32_t last_second_seen = UINT32_MAX;
  static uint32_t last_draw_second = UINT32_MAX;
  const uint32_t now_second = millis() / 1000U;
  if (now_second == last_second_seen) {
    return;
  }
  last_second_seen = now_second;

  // if (last_draw_second != UINT32_MAX && (now_second - last_draw_second) < DISPLAY_UPDATE_INTERVAL_SEC) {
  //   return;
  // }

  const uint32_t display_second = now_second % 60U;
  Serial.printf("Sec: %lu\n", static_cast<unsigned long>(display_second));

  last_draw_second = now_second;
  draw_counter_value(display_second);

}
#endif

void setup_boot_button() {
  pinMode(PIN_BOOT_BUTTON_CFG, INPUT_PULLUP);
}

void handle_boot_button() {
  static int last_raw_state = HIGH;
  static int stable_state = HIGH;
  static uint32_t last_change_ms = 0;
  const uint32_t debounce_ms = 30;

  const int raw_state = digitalRead(PIN_BOOT_BUTTON_CFG);
  if (raw_state != last_raw_state) {
    last_raw_state = raw_state;
    last_change_ms = millis();
  }

  if ((millis() - last_change_ms) < debounce_ms) {
    return;
  }

  if (stable_state == raw_state) {
    return;
  }

  stable_state = raw_state;
  if (stable_state == LOW) {
    counter_text_red = !counter_text_red;
    Serial.printf("Counter color switched to: %s\n", counter_text_red ? "RED" : "BLACK");
    draw_counter_value(current_display_second);
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("Starting EPD demo...");
  Serial.printf("Mode USE_LVGL=%d\n", USE_LVGL);
  setup_boot_button();
  setup_rgb_led();
  log_graphics_backend();
  setup_display();
#if USE_LVGL
  setup_lvgl();
  build_ui();
  draw_counter_value(0);
  lv_timer_handler();
#else
  draw_counter_value(0);
#endif
}

void loop() {
  sync_rgb_led_with_epd_busy();
  handle_boot_button();
  update_counter();

#if USE_LVGL
  lv_timer_handler();
#endif

  delay(5);
}
