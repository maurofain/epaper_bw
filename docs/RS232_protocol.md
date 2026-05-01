# Protocollo RS232 verso la scheda Master (MH1001)

## 1. Scopo dell’interfaccia
La connessione RS232 tra la scheda e la scheda Master MH1001 serve a:
- inoltrare i codici letti dallo scanner verso la MH1001;
- ricevere comandi e stringhe da visualizzare sul display;
- gestire richieste di refresh e controllo LED.

## 2. Collegamento fisico
La comunicazione seriale avviene su:
- `TXD0` → uscita seriale dalla scheda ESP32-S2-MINI-2-N4
- `RXD0` → ingresso seriale verso la scheda ESP32-S2-MINI-2-N4

Questa interfaccia passa attraverso un chip `MAX232` che converte i livelli TTL del microcontroller ai livelli RS232 richiesti dalla MH1001.

## 3. Funzionalità della porta RS232
La porta `TXD0/RXD0` è dedicata a due ruoli principali:
- inoltro del codice scanner verso la MH1001;
- ricezione di pacchetti di controllo e di visualizzazione.

## 4. Formato del pacchetto
Il primo byte del pacchetto determina il tipo di dato:
- `0x00` = comando
- `0x01` = stringa da visualizzare
- `0x02` = comando di controllo LED

---

## 5. Pacchetto `0x01`: visualizzazione stringa

Formato base:
- `0x01`
- `LEN`
- `Ch0, Ch1, ..., ChN`

Dove:
- `LEN` = numero di caratteri della stringa (fino a 127)
- `Ch0..ChN` = caratteri ASCII da visualizzare

Note:
- È ammesso un carattere `CR` per indicare una rottura di riga.
- Prima di pubblicare la nuova stringa, la scheda esegue un refresh parziale per cancellare il testo precedente.

### 5.1 Formato esteso con posizionamento e font

Se il byte `LEN` vale `0xFF`, il pacchetto usa il formato esteso:
- `0x01`
- `0xFF`
- `font#`
- `pos_x`
- `pos_y`
- `Ch0, Ch1, ..., ChN`
- `0x00`

Dove:
- `font#` = valore da `1` a `15` (vedere sezione 11.3 per lista completa)
- `pos_x`, `pos_y` = coordinate in pixel riferite all’angolo superiore sinistro della finestra di visualizzazione
- la stringa è terminata da `0x00`

Questo formato permette di inviare testo con font e posizione esplicita.

### 5.2 Regole di layout e font
La stringa ricevuta deve essere impaginata sul display con queste regole:
- scegliere il font più grande possibile per contenere l’intero testo;
- ridurre la dimensione del font se necessario fino a che il testo si adatta;
- costruire le righe raggruppando quante più parole possibili;
- se il testo non entra con tutte le righe disponibili, troncare lasciando gli ultimi 4 caratteri in ` ...`.
- il carattere § a inizio testo effettua la cancellazione del pannello ma non viene stampato

Font disponibili (15 totali):
- **Font 1-3 (Montserrat)**: 14pt, 28pt, 48pt
- **Font 4-11 (GoogleSans)**: 10pt, 15pt, 20pt, 28pt, 40pt, 60pt, 100pt, 140pt
- **Font 12-15 (GoogleSansBold)**: 40pt, 60pt, 100pt, 140pt

### 5.3 Modificatori
##### Modificatori di testo (Comandi `t`, `a`)

All'inizio del testo puoi aggiungere modificatori:
- **§**: Effettua il refresh parziale del display (non viene stampato)
- **ç**: Abilita il font bold (se disponibile)
- **£**: Seleziona famiglia di font Montserrat anziché GoogleSans

---

## 6. Pacchetto `0x00`: comando

I comandi hanno il formato:
- `0x00`, `0xAA` → inizializza la connessione con il master: fino all'arrivo di questo pacchetto tutto ciò che arriva sulla seriale va ignorato
- `0x00`, `0xEE` → interrompe la connessione con il master: fino all'arrivo di un nuovo pacchetto 0x00 0xAA tutto ciò che arriva sulla seriale va ignorato
- `0x00`, `0x00` → refresh totale del display
- `0x00`, `0x01` → richiesta di attivazione dello scanner e refresh totale del display
- `0x00`, `0x02` → scanner ON
- `0x00`, `0x03` → scanner OFF
- `0x00`, `0x04` → attiva sfondo bianco e testo nero (default)
- `0x00`, `0x05` → attiva sfondo nero e testo bianco
- `0x00`, `0xBB` → esegue un reboot del esp32s2
- `0x00`, `0xFF` → mostra il logo sull'e-paper

> Il secondo byte specifica il tipo di comando all’interno della categoria `0x00`.

---

## 7. Pacchetto `0x02`: controllo LED WS2812B

Formato:
- `0x02`
- 12 byte dati: `l1r, l1g, l1b, l2r, l2g, l2b, l3r, l3g, l3b, l4r, l4g, l4b`

Dove:
- `lNr` = valore rosso per LED N
- `lNg` = valore verde per LED N
- `lNb` = valore blu per LED N

Questo comando imposta i 4 LED WS2812B collegati alla scheda.

---

## 8. Flusso operativo tipico

1. La scheda riceve un pacchetto RS232 su `RXD0`.
2. Legge il primo byte per identificare il tipo:
   - `0x00` → comando
   - `0x01` → stringa
   - `0x02` → LED
3. Esegue l’azione corrispondente:
   - refresh del display;
   - visualizzazione testo con gestione font e line break;
   - aggiornamento colori LED.
4. Se è una stringa, prima cancella il testo precedente con refresh parziale, poi pubblica il nuovo contenuto.

---

## 9. Note di integrazione con il sistema
- Il circuito prevede cinque interfacce, ma la RS232 verso MH1001 è solo una di queste.
- L’interfaccia `TXD0/RXD0` è esclusivamente quella che va al `MAX232` e da lì alla MH1001.
- La scheda deve anche gestire la comunicazione dello scanner Newland N1-W, ma questo avviene in un layer diverso: i codici letti dallo scanner vengono semplicemente ritrasmessi su `TXD0` verso la MH1001.

---

## 10. Raccomandazioni di implementazione
- usare un baud rate RS232 stabile e sincronizzato con la MH1001;
- validare la lunghezza `LEN` prima di leggere i caratteri;
- gestire eventuali caratteri di controllo come `CR` per il ritorno a capo;
- isolare fisicamente il collegamento con `MAX232` per preservare i livelli di segnale corretti.

---

## 11. Tester RS232 Protocol (`scripts/rs232_protocol_test.py`)

Il tester è un'interfaccia curses interattiva per testare e debuggare la comunicazione RS232 con la scheda.

### 11.1 Comandi disponibili dal menu principale

| Comando | Descrizione |
|---------|-------------|
| `c` | Seleziona comando 0x00 (refresh, scanner, tema, reboot, logo) |
| `i` | Invia comando init: `0x00 0xAA` |
| `r` | Invia comando reboot: `0x00 0xBB` |
| `x` | Invia comando disconnect: `0x00 0xEE` |
| `a` | Mostra elenco dei 15 font disponibili |
| `t` | Testo auto: seleziona automaticamente il font in base alla lunghezza |
| `e` | Testo esteso: consente di scegliere font (1-15), posizione x,y e testo |
| `l` | Controllo LED: imposta colori RGB per i 4 LED WS2812B |
| `w` | Watch: visualizza testo con secondi aggiornati ogni 1s (con scelta del font) |
| `d` | Countdown: visualizza testo con contador da 60 a 0 (con scelta del font) |
| `z` | Font demo: cicla attraverso tutti i 15 font mostrando il nome e il numero "19" |
| `f` | Toggle log visibility: mostra/nasconde il riquadro dei log seriali |
| `q` | Esci dal tester |

### 11.2 Comandi 0x00 (sottomenu `c`)

| Comando | Codice | Descrizione |
|---------|--------|-------------|
| `0` | 0x00 0x00 | Refresh totale del display |
| `1` | 0x00 0x01 | Attiva scanner ON + refresh totale |
| `2` | 0x00 0x02 | Scanner ON |
| `3` | 0x00 0x03 | Scanner OFF |
| `4` | 0x00 0x04 | Tema normale (sfondo bianco, testo nero) |
| `5` | 0x00 0x05 | Tema invertito (sfondo nero, testo bianco) |
| `b` | 0x00 0xBB | Reboot ESP32-S2 |
| `f` | 0x00 0xFF | Mostra logo sull'e-paper |

### 11.3 Selezione font (Comandi `e`, `w`, `d`)

I font sono disponibili con indici 1-15:
- **1-3**: Montserrat 14pt, 28pt, 48pt
- **4-11**: GoogleSans 10pt-140pt (incrementi proporzionali)
- **12-15**: GoogleSansBold 40pt-140pt

Selezione con codici:
- Inserisci numero `1-15` per scegliere il font esplicito
- Inserisci `a`, `b`, `c`, `d` come scorciatoia per font 12-15 (bold)

### 11.4 Modificatori di testo (Comandi `t`, `a`)

All'inizio del testo puoi aggiungere modificatori:
- **§**: Effettua il refresh parziale del display (non viene stampato)
- **ç**: Abilita il font bold (se disponibile)
- **£**: Seleziona famiglia di font Montserrat anziché GoogleSans

Esempio: `ç§Il mio testo` → visualizza in bold con refresh parziale

### 11.5 Auto-init on boot

Quando il tester viene avviato e riceve il messaggio `[M] init completed` dalla scheda, invia automaticamente il comando init (`0x00 0xAA`) senza richiedere intervento dell'utente.

### 11.6 Font demo (Comando `z`)

Cicla attraverso tutti i 15 font, visualizzando per ogni font:
- Nome del font in alto a sinistra con font Montserrat 14pt
- Numero "19" centrato nel font di destinazione
- Intervallo di 4 secondi tra le transizioni
- Full refresh tra i font per evitare artefatti di visualizzazione

### 11.7 Watch e Countdown (Comandi `w`, `d`)

**Watch (`w`)**: 
1. Richiede scelta del font (1-15, a-d per bold)
2. Richiede testo da visualizzare
3. Visualizza il testo con i secondi aggiornati ogni 1 secondo
4. Qualsiasi tasto ferma il watch

**Countdown (`d`)**:
1. Richiede scelta del font (1-15, a-d per bold)
2. Richiede testo da visualizzare
3. Visualizza il testo con un contador che scende da 60 a 0 ogni 1 secondo
4. A 15 secondi attiva il tema invertito (nero su bianco)
5. A 0 secondi ripristina il tema normale
6. Qualsiasi tasto ferma il countdown

### 11.8 Interfaccia visuale

Il tester presenta due riquadri:
- **Sinistra**: Menu, comandi disponibili, prompt, status
- **Destra**: Log dei dati ricevuti dalla seriale (toggle con `f`)

Ogni riga ricevuta dalla seriale viene loggata con timestamp nel formato `[HH:MM:SS] RX: ...`
