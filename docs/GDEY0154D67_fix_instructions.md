# Correzioni per utilizzo pannello GDEY0154D67

Questo documento descrive le modifiche consigliate al firmware `test_epaper_bw` per far funzionare correttamente il pannello GDEY0154D67.

## 1. Controlla il pannello e la connessione

- Il pannello `GDEY0154D67` è un 1.54" 200x200 e-paper.
- Verifica che `EPD_WIDTH` e `EPD_HEIGHT` siano effettivamente impostati su `200` x `200` nel codice corrente. Nel file `src/main.cpp` questo accade se non è definito `USE_EPD_GDEY1085F51`.
- Allinea i pin del firmware alle connessioni hardware reali del display.
  - L’esempio funzionante usa: MOSI=4, SCK=5, CS=6, DC=7, RST=15, BUSY=16.
  - Il firmware attuale usa `pin_config.h` con: `CS=36`, `DC=35`, `RST=34`, `BUSY=33`, `SCK=37`, `MOSI=38`.
- Se usi il display collegato come nell’esempio, modifica `include/pin_config.h` oppure la cablatura hardware.

## 2. Controlla la sequenza di inizializzazione

Nel firmware attuale, la funzione centrale è `epdInitSequence()` in `src/main.cpp`.

### Verifica questi punti:

- `epdPowerOn()` viene chiamato solo se `PIN_EPD_PWR` è definito. Se il display richiede alimentazione tramite pin, non lasciare `PIN_EPD_PWR = -1`.
- `epdReset()` deve essere eseguito con:
  - RST = HIGH 200 ms
  - RST = LOW 10 ms
  - RST = HIGH 200 ms
- `waitUntilIdle()` deve usare la polarità corretta del pin BUSY. Per GDEY0154D67 il livello attivo è tipicamente `LOW` (0). Se il tuo BUSY è invece attivo `HIGH`, cambia `EPD_BUSY_ACTIVE_LEVEL` in `1`.

### Sequenza di init da verificare/correggere

Nel `epdInitSequence()` attuale sono inviati questi comandi:

- `0x12` SWRESET
- `0x01` Driver output control
- `0x11` Data entry mode
- `0x44` Ram X address start/end
- `0x45` Ram Y address start/end
- `0x3C` Border waveform
- `0x18` Temperature sensor
- `0x4E` Set RAM X address
- `0x4F` Set RAM Y address

Questa sequenza è valida come base, ma deve essere comparata con il driver della libreria Arduino `GxEPD2` per `GDEY0154D67`. Se il display non risponde, la correzione più probabile riguarda:

- parametri di `0x01` (ordine bit / orientamento)
- valori di `0x44` / `0x45` per i limiti RAM X/Y
- polarità BUSY e timing nel `waitUntilIdle()`

## 3. Correggi il flush LVGL / invio dati al display

Il vero punto critico è la funzione di trasferimento della grafica in `src/main.cpp`:
- `epd_flush_cb()`
- `sendEpdPartialImage()`

### Cosa verificare

- `epd_flush_cb()` deve inviare i comandi corretti prima e dopo il trasferimento dei pixel.
- `sendEpdPartialImage()` deve impostare l’area parziale con `0x44`, `0x45`, `0x4E`, `0x4F` e inviare i dati con `0x24`.
- Dopo i dati, deve eseguire `0x22` + `0xF7` e quindi `0x20`.
- Assicurati che il mapping bit/pixel rispetti il formato del pannello: nei comandi attuali `1` viene inviato come bianco e `0` come nero.

### Passaggi consigliati

1. prima di usare LVGL, verifica che il display risponda con un semplice test full-screen:
   - `epdClearScreen()` per una screen bianca
   - `epdBlackScreen()` per una screen nera
   - `epdFullRefresh()` per aggiornare il pannello
2. se questi test funzionano, passa alla parte LVGL e controlla la funzione di flush.
3. aggiungi log in `epd_flush_cb()` e in `sendEpdPartialImage()` per verificare dimensioni area e numero di byte trasmessi.

## 4. Modifiche consigliate al codice

### Pin e SPI
- In `include/pin_config.h`:
  - verifica `PIN_SPI_SCK` e `PIN_SPI_MOSI`.
  - se usi l’esempio Arduino, porta `PIN_EPD_CS=6`, `PIN_EPD_DC=7`, `PIN_EPD_RST=15`, `PIN_EPD_BUSY=16`, `PIN_SPI_SCK=5`, `PIN_SPI_MOSI=4`.
- Se il display è alimentato tramite pin PWR, abilita `PIN_EPD_PWR` e assicurati che `epdPowerOn()` lo porti ad `1`.

### Reset e attesa BUSY
- In `src/main.cpp` imposta `EPD_BUSY_ACTIVE_LEVEL` al valore corretto.
- `waitUntilIdle()` deve leggere il BUSY pin fino all’uscita dal livello di busy.

### Inizializzazione display
- Confronta il driver della libreria Arduino e aggiorna `epdInitSequence()` se necessario.
- Se il pannello richiede comandi aggiuntivi di power setting, booster soft-start o VCOM, aggiungili nella fase di init.

### Flush grafica
- In `src/main.cpp`:
  - verifica che `epd_flush_cb()` chiami sempre `sendEpdCommand(0x11); sendEpdData(0x01);` prima di inviare i pixel.
  - conferma che i comandi `0x24` e `0x26` vengano inviati con il corretto insieme di byte.
  - controlla che `lv_disp_flush_ready(drv)` avvenga solo dopo il `waitUntilIdle()` se serve.

## 5. Test di verifica

Esegui questi test uno alla volta:

1. `epdReset()` + `waitUntilIdle()` senza trasferire dati: il BUSY deve cambiare stato.
2. `epdInitSequence()` da sola: il display deve accettare i comandi senza errori.
3. `epdClearScreen()` e `epdBlackScreen()`: lo schermo deve passare bianco/nero.
4. `epdFullRefresh()` dopo il trasferimento del buffer.
5. solo dopo il test full-screen, attiva LVGL e verifica `epd_flush_cb()`.

## 6. Rapida regola d’oro

Se vuoi ridurre i problemi, usa prima un driver diretto per il display e-paper invece di integrare subito LVGL. Una volta validata la sequenza di init e il refresh, puoi riportare i pixel da LVGL.

## Note aggiuntive

- Questo firmware è più “bare metal” del progetto Arduino di riferimento: la libreria `GxEPD2` gestisce internamente tutta l’inizializzazione e l’update.
- Quando il display non risponde, il problema è quasi sempre nella sequenza di init o nel BUSY pin.
- Se vuoi, puoi anche usare direttamente la libreria `GxEPD2` su ESP-IDF/arduino-esp32 come riferimento per i comandi esatti del GDEY0154D67.
