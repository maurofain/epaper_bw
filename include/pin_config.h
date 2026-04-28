#pragma once

#include <cstdint>

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#ifndef PIN_BOOT_BUTTON
#define PIN_BOOT_BUTTON 0
#endif
#ifndef PIN_RGB_LED
#define PIN_RGB_LED 3
#endif
#ifndef PIN_EPD_CS
#define PIN_EPD_CS 36
#endif
#ifndef PIN_EPD_CS2
#define PIN_EPD_CS2 -1
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
#ifndef PIN_EPD_PWR
#define PIN_EPD_PWR -1
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
#define PIN_QR_BCRES 45
#endif
#ifndef PIN_QR_BCTRIG
#define PIN_QR_BCTRIG 42
#endif

constexpr int8_t PIN_BOOT_BUTTON_CFG = PIN_BOOT_BUTTON;
constexpr int8_t PIN_RGB_LED_CFG = PIN_RGB_LED;
constexpr int8_t PIN_EPD_CS_CFG = PIN_EPD_CS;
constexpr int8_t PIN_EPD_CS2_CFG = PIN_EPD_CS2;
constexpr int8_t PIN_EPD_DC_CFG = PIN_EPD_DC;
constexpr int8_t PIN_EPD_RST_CFG = PIN_EPD_RST;
constexpr int8_t PIN_EPD_BUSY_CFG = PIN_EPD_BUSY;
constexpr int8_t PIN_EPD_PWR_CFG = PIN_EPD_PWR;
constexpr int8_t PIN_SPI_SCK_CFG = PIN_SPI_SCK;
constexpr int8_t PIN_SPI_MOSI_CFG = PIN_SPI_MOSI;
constexpr int8_t PIN_SPI_MISO_CFG = PIN_SPI_MISO;
constexpr int8_t PIN_SCANNER_TX_CFG = PIN_SCANNER_TX;
constexpr int8_t PIN_SCANNER_RX_CFG = PIN_SCANNER_RX;
constexpr int8_t PIN_RX_ESP_CFG = PIN_RX_ESP;
constexpr int8_t PIN_TX_ESP_CFG = PIN_TX_ESP;
constexpr int8_t PIN_QR_BCRES_CFG = PIN_QR_BCRES;
constexpr int8_t PIN_QR_BCTRIG_CFG = PIN_QR_BCTRIG;

#endif // PIN_CONFIG_H
