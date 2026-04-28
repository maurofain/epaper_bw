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

## Pin flat scanner (lato ESP32)

| Pin flat N1-W | Segnale scanner | Collegamento lato ESP32 | GPIO ESP32 | Stato |
|---|---|---|---|---|
| 1 oppure 11 | GND | GND | - | obbligatorio |
| 2 | EXT_TRIG# | PIN_QR_BCTRIG | 42 | opzionale (trigger HW) |
| 3 | EXT_RST# | PIN_QR_BCRES | 45 | opzionale (reset HW) |
| 4 | EXT_DSF | non mappato nel firmware attuale | - | opzionale |
| 5 | EXT_BUZ | non mappato nel firmware attuale | - | opzionale |
| 6 | EXT_LIGHT | non mappato nel firmware attuale | - | opzionale |
| 9 | TTL232_TX | PIN_SCANNER_RX | 18 | obbligatorio per UART |
| 10 | TTL232_RX | PIN_SCANNER_TX | 17 | obbligatorio per UART |
| 12 oppure 13 | VCC | 3.3V | - | obbligatorio se alimentato dalla scheda |

## Build / Upload

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Attivare il log seriale su porta USB (USB CDC)

Per visualizzare i log della app sulla porta USB nativa dell'ESP32-S2 (tipicamente `/dev/ttyACM0`):

1. Verifica in `sdkconfig` che siano attive queste opzioni:
	- `CONFIG_ESP_CONSOLE_USB_CDC=y`
	- `CONFIG_ESP_CONSOLE_USB_CDC_SUPPORT_ETS_PRINTF=y`
	- `CONFIG_ESP32S2_KEEP_USB_ALIVE=y`

2. Compila il firmware:

```bash
idf.py build
```

3. Collega la porta USB dati della scheda e identifica la porta seriale (es. `/dev/ttyACM0`).

4. Flash + monitor sulla stessa porta:

```bash
idf.py -p /dev/ttyACM0 flash monitor
```

5. Riavvia la scheda e verifica i log di avvio (`app_main entry`, `init step: ...`).

Se la porta USB scompare dopo il reset:
- scollega/ricollega il cavo USB dati;
- verifica che non sia in uso da altri monitor;
- rilancia `idf.py -p /dev/ttyACM0 monitor`.

## Note

- Il parser seriale supporta i comandi 0x00, 0x01, 0x02 e la visualizzazione di stringhe.
- Il progetto ora è focalizzato su ESP32-S2 e display 1.54".

## Test scanner QR

Funzioni di test/diagnostica scanner QR in firmware:

- `scannerSerialSelfTest` — definita in `src/scanner_control.cpp`, dichiarata in `include/scanner_control.h`, invocata in `src/main.cpp`
- `runScannerUartTxBurstDiagnostic` — definita in `src/main.cpp`, invocata in modalità diagnostica TX
- `startScannerTxSquareWave` — definita in `src/main.cpp`, invocata in modalità onda quadra TX
- `scannerTxWaveCallback` — callback di supporto alla generazione dell’onda quadra TX

Test scanner da PC (script Python):

- `run_listen_only` — definita in `scripts/scanner_pc_probe.py`
- `run_probe` — definita in `scripts/scanner_pc_probe.py`
- `send_and_read` — helper usato dai test di probe in `scripts/scanner_pc_probe.py`

## Inizializzazione scanner attuale

La sequenza di inizializzazione attuale è gestita tra `src/main.cpp` e `src/scanner_control.cpp`.

1. In `app_main()` viene inizializzata la UART dello scanner (`UART_SCANNER`) a 9600 baud, quindi viene eseguito `setupScannerPins()`.
2. `setupScannerPins()` configura i pin di controllo scanner:
   - in modalità seriale mantiene il pin `PIN_QR_BCRES_CFG` (RESET) come uscita e lo porta a livello logico alto, mentre il pin `PIN_QR_BCTRIG_CFG` (TRIGGER) viene configurato come ingresso;
   - se abilitato `SCANNER_CONTROL_USE_TRIGGER`, entrambi i pin di controllo vengono configurati come uscite e portati a livello alto.
3. Dopo la configurazione dei pin, `initializeScanner()` viene chiamata per marcare lo scanner come inizializzato, attivare eventuale configurazione trigger aggiuntiva e attendere 50 ms di stabilizzazione.
4. Se `SCANNER_SERIAL_SELF_TEST_ENABLE` è attivo, `app_main()` avvia `scannerSerialSelfTest(UART_SCANNER)` per il controllo seriale dello scanner; in questa build la modalità `SCANNER_LISTEN_ONLY_MODE_ENABLE` è impostata per ascoltare continuamente i dati scanner e loggarli in HEX/ASCII.

Questa è la sequenza reale di inizializzazione usata dal firmware in questa versione del progetto.
