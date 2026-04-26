## Plan: Adattamento firmware per nuova scheda EpaperQr

TL;DR: Convertire l'attuale demo ESP32-S3 + e-paper in un firmware per la nuova scheda EpaperQr con display Waveshare 1.54" 200x200, seriale verso MH1001, controllo scanner Newland N1-W, gestione LED WS2812B e selezione dinamica dei font.

**Steps**
1. Aggiornare la configurazione di build e pin hardware.
   - Modificare `CMakeLists.txt` per includere le librerie corrette e impostare i pin della nuova scheda.
   - Verificare e cambiare il driver GxEPD2 da `GxEPD2_213_Z98c` a quello corretto per GDEY0154D67/1.54" 200x200.
   - Disabilitare LVGL se non necessario e usare il backend Adafruit GFX/GxEPD2 diretto.

2. Mappare i pin del nuovo hardware in `src/main.cpp`.
   - Definire costanti per EP BUSY, EP RESET, EP DC, EP CS, EP SCK, EP SDI, QR BCRES, QR BCTRIG, LED WS2812B, TXesp, RXesp.
   - Aggiornare i pin SPI e il driver display con i valori forniti in TODO.
   - Aggiornare `Adafruit_NeoPixel rgb_led` a 4 pixel.

3. Aggiungere i font richiesti e la logica di selezione.
   - Inserire in `src/ui/fonts/` tutti i font disponibili: `GoogleSans140.c`, `GoogleSans100.c`, `GoogleSans60.c`, `GoogleSans50.c`, `GoogleSans35.c`, `GoogleSans20.c`, `GoogleSans15.c`, `GoogleSans10.c`.
   - Considerare che le dimensioni `140`, `100` e `60` sono da usare solo per testo numerico.
   - Creare o aggiornare una dichiarazione di font (`src/ui/fonts/user_fonts.h` o in `main.cpp`).
   - Implementare la logica di scelta font in base a conteggio caratteri e digit/alfanumerico, come richiesto dal TODO.

4. Implementare la ricezione seriale e il parser dei comandi.
   - Configurare la seriale che riceve da RXesp/TXesp (UART su GPIO 39/40) per leggere pacchetti.
   - Supportare i comandi:
     - `0x00 0x00` = refresh totale del display.
     - `0x00 0x01` = attivazione scanner + refresh totale display.
     - `0x01 LEN data...` = stringa da visualizzare, con CR come ritorno a capo.
     - `0x02 [12 byte]` = valori RGB per 4 LED WS2812B.
   - Gestire un buffer e la ricezione completa prima di eseguire il comando.

5. Implementare la logica di visualizzazione testuale.
   - Creare funzioni per:
     - calcolare righe e word-wrap per il font corrente;
     - provare il font più grande e scendere finché il testo non entra;
     - troncare con ` ...` se il testo non ci sta.
   - Usare partial refresh e cancellare il testo precedente prima di disegnare il nuovo.
   - Gestire la rottura di riga esplicita con CR.

6. Controllo WS2812B.
   - Modificare la gestione LED per usare 4 pixel.
   - Applicare i 12 byte ricevuti per impostare i colori di ciascun LED.

7. Gestione scanner Newland N1-W e forwarding verso MH1001.
   - Implementare la fase di inizializzazione scanner con comandi di setup.
   - Creare una routine che resta in attesa di letture scanner e ritrasmette il codice su TXesp.
   - Collegare QR BCRES/BCTRIG secondo la specifica hardware, se necessario.

8. Verifica e test.
   - Compilare e testare con `idf.py build`.
   - Verificare l'inizializzazione display e i pin hardware.
   - Testare i comandi seriali con pacchetti reali e il corretto funzionamento WS2812B.
   - Validare il comportamento di line-wrapping e truncation.

**Relevant files**
- `CMakeLists.txt` — aggiornare board, build flags e pin di compilazione.
- `src/main.cpp` — adattare driver display, pin, parser seriale, font selection, LED e scanner.
- `src/ui/fonts/` — aggiungere i font GoogleSans richiesti.
- `include/lv_conf.h` — probabilmente non serve modificare se si disabilita LVGL.

**Verification**
1. Build success con `idf.py build`.
2. Flash e log di avvio per confermare il driver e i pin.
3. Invio di pacchetti `0x01 LEN ...` e verifica che il testo appaia sul display con font corretto.
4. Invio di pacchetti `0x02 ...` e verifica colori WS2812B.
5. Test scanner init + forwarding verso MH1001.

**Decisions / Assumptions**
- Il firmware userà il backend diretto GxEPD2/Adafruit GFX senza LVGL.
- I pin dati serie RXesp/TXesp saranno gestiti come UART hardware separato dal debug USB.
- La gestione scanner richiede conferma del protocollo Newland N1-W, ma la struttura va preparata.
- I font saranno caricati come file `.c` in `src/ui/fonts/` e dichiarati via header.
