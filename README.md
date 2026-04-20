# ESP32-S3 + ePaper 2.13" 3 colori (bianco/nero/rosso) con LVGL

Progetto base PlatformIO per:
- ESP32-S3-WROOM
- Display ePaper 2.13" tri-color
- LVGL
- Contasecondi dal boot

## Struttura

- `platformio.ini`: configurazione build
- `include/lv_conf.h`: configurazione LVGL
- `src/main.cpp`: logica display + UI + contasecondi
- `src/ui/fonts/user_font_70.h`: dichiarazione font personalizzato

## Font 70px personalizzato

1. Copia il tuo font in formato `.c` in `src/ui/fonts/` (qui è `GoogleSans70.c`).
2. Verifica che il simbolo esposto sia `GoogleSans70`.
3. In `platformio.ini`, imposta:

```ini
build_flags =
  -DLV_CONF_INCLUDE_SIMPLE
  -DUSE_USER_FONT_70=1
```

Se `USE_USER_FONT_70=0`, usa il fallback `lv_font_montserrat_48`.

## Pin e modello pannello

Nel file `src/main.cpp`:
- aggiorna i pin SPI e segnali del display (`PIN_EPD_*`, `PIN_SPI_*`)
- se il tuo pannello non è `GxEPD2_213_Z19c`, sostituisci il tipo corretto GxEPD2

## Build / Upload

```bash
pio run
pio run -t upload
pio device monitor
```

## Note pratiche

- L'e-paper tri-color è lenta: un refresh al secondo è funzionale per test, ma non fluido.
- Se vuoi, nel prossimo step possiamo ottimizzare il refresh (aree piccole / cadenza diversa).
