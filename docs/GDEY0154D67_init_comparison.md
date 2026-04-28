# Confronto inizializzazione display GDEY0154D67

Questo documento confronta l’inizializzazione del display e-paper nel firmware corrente con l’inizializzazione di riferimento della libreria Arduino per `epd1in54b_V2`.

## Contesto

- Pannello: `GDEY0154D67` (Waveshare 1.54" 200x200, GoodDisplay)
- Firmware attuale: `test_epaper_bw` con backend LVGL/Epaper
- Riferimento: `1P/MicroHard/test_epaper/E-Paper_code/Arduino/epd1in54b_V2`

## Confronto funzionale

| Funzione / fase | Firmware `test_epaper_bw` | Riferimento Arduino `epd1in54b_V2` |
|---|---|---|
| Pin e segnale | imposta CS, DC, RST, BUSY e opzionale CS2; supporto PWR aggiunto ma dipende da pin configurato | imposta CS, DC, RST, BUSY, PWR |
| Configurazione GPIO | pin GPIO configurati in output per CS/DC/RST/PWR e input per BUSY | pinMode per tutti i pin e alimentazione PWR |
| Abilitazione alimentazione | implementata via `epdPowerOn()` se `PIN_EPD_PWR` è definito | `DigitalWrite(PWR_PIN, 1)` |
| Inizializzazione SPI | implementata con `spi_bus_initialize()` e `spi_bus_add_device()` | `SPI.begin(); SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));` |
| Reset hardware | implementato `epdReset()` con HIGH 200ms, LOW 10ms, HIGH 200ms | `Reset()` con HIGH 200ms, LOW 10ms, HIGH 200ms |
| Attesa BUSY | implementata `waitUntilIdle()` ma con polarità probabilmente invertita | `WaitUntilIdle()` legge `BUSY_PIN` e attende il livello corretto (tipicamente LOW=busy, HIGH=idle) |
| Reset software | implementato `SendCommand(0x12)` SWRESET | `SendCommand(0x12)` SWRESET |
| Impostazione driver | implementato `0x01` + `0xC7 0x00 0x01` | `SendCommand(0x01); SendData(0xC7); SendData(0x00); SendData(0x01);` |
| Modalità entry dati | implementato `SendCommand(0x11); SendData(0x01)` | `SendCommand(0x11); SendData(0x01);` |
| Set Ram X/Y bounds | implementato `0x44`, `0x45` con parametri X/Y | `0x44`, `0x45` con parametri X/Y |
| Border waveform | implementato `SendCommand(0x3C); SendData(0x05)` | `SendCommand(0x3C); SendData(0x05);` |
| Sensore temperatura | implementato `SendCommand(0x18); SendData(0x80)` | `SendCommand(0x18); SendData(0x80);` |
| Contatori RAM | implementato `0x4E`, `0x4F` | `0x4E`, `0x4F` impostano puntatori RAM |
| Inizializzazione effettiva | sequenza di init SPI e comandi di setup ora implementata | sequenza completa di inizializzazione SPI |
| Refresh / immagine | `epd_flush_cb()` ora invia un’area LVGL al pannello e richiama `DISPLAY_REFRESH` | `DisplayClear()` / `DisplayFrame(...)` inviano dati e refresh |

## Interazione con LVGL

- Il firmware usa LVGL come backend grafico, ma non esegue il rendering diretto sul display e-paper.
- `epd_flush_cb()` chiama solo `lv_disp_flush_ready(drv)` senza trasferire i pixel a SPI o ricostruire la memoria del pannello.
- LVGL gestisce un buffer software, ma senza una catena di trasmissione al controller del display, il pannello rimane fermo.
- Per usare LVGL con questo pannello, serve una funzione di flush che traduca il framebuffer LVGL in comandi `SendCommand`/`SendData` e faccia il `DISPLAY_REFRESH`.

## Conclusione

L’attuale firmware non esegue la vera inizializzazione del pannello e-paper. La sequenza Arduino di riferimento mostra che il display richiede:

- alimentazione PWR attiva
- inizializzazione SPI
- reset hardware
- attesa BUSY
- comandi di setup del controller
- puntatori RAM e configurazione waveform

Per far funzionare il `GDEY0154D67`, il firmware deve implementare una sequenza di avvio simile a quella in `epd1in54b_V2`.

## Raccomandazioni

1. aggiungere un driver SPI completo per il display
2. implementare il reset hardware e la gestione del pin PWR
3. inviare i comandi di inizializzazione in sequenza
4. attendere BUSY prima di procedere
5. trasferire dati di immagine con `DISPLAY_REFRESH`
