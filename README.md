# ESP32-S2 + E-Paper 1.54" 200x200 con LVGL

Firmware per la scheda EpaperQr basata su ESP32-S2-MINI-2-N4 e display Waveshare 1.54" 200x200 (GDEY0154D67).

## Struttura

- `CMakeLists.txt`: configurazione build per ESP-IDF
- `include/lv_conf.h`: configurazione LVGL
- `src/main.cpp`: logica display, parser seriale, gestione LED e scanner
- `src/ui/fonts/user_fonts.h`: inclusione dei font GoogleSans disponibili

## Font utilizzati

I font disponibili in `src/ui/fonts/` sono:
- `GoogleSans140.c` (numerico)
- `GoogleSans100.c` (numerico)
- `GoogleSans60.c` (numerico)
- `GoogleSans50.c`
- `GoogleSans35.c`
- `GoogleSans20.c`
- `GoogleSans15.c`
- `GoogleSans10.c`

Le dimensioni 140, 100 e 60 sono destinate all'uso numerico.

## Pin e display

Nel file `src/main.cpp` sono configurati i pin per il nuovo hardware:
- `PIN_EPD_*` e `PIN_SPI_*` per il display e-paper
- `PIN_RX_ESP` / `PIN_TX_ESP` per la seriale verso MH1001
- `PIN_SCANNER_RX` / `PIN_SCANNER_TX` per lo scanner
- `PIN_QR_BCRES` / `PIN_QR_BCTRIG` per il controllo scanner
- `PIN_RGB_LED` per i WS2812B

## Build / Upload

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Note

- Il parser seriale supporta i comandi 0x00, 0x01, 0x02 e la visualizzazione di stringhe.
- Il progetto ora è focalizzato su ESP32-S2 e display 1.54".
