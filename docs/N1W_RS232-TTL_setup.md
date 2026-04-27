# Configurazione scanner N1-W per connessione diretta a ESP32-S2

## 1. Obiettivo
Questa guida spiega come impostare il lettore scanner N1-W per una connessione seriale TTL diretta con un ESP32-S2, senza la necessità di un convertitore RS232.

## 2. Connessione fisica
Il modulo N1-W usa un connettore FPC a 13 pin. Per la comunicazione TTL-232 direttamente su ESP32-S2 bisogna utilizzare i pin dedicati:

- Pin 9: `TTL232_TX` → ingresso RX dell’ESP32-S2
- Pin 10: `TTL232_RX` → uscita TX dell’ESP32-S2
- Pin 1 o Pin 11: `GND` → massa comune
- Pin 12 / Pin 13: `VCC` → alimentazione 3.3 V (se richiesta dalla configurazione hardware)

> Il segnale `TTL232` è a livello 3.3 V, quindi può essere collegato direttamente ai pin UART dell’ESP32-S2 senza un convertitore di livello.

## 3. Impostazioni seriale richieste
Il manuale N1-W indica che l’interfaccia seriale deve essere configurata con i seguenti parametri di comunicazione:

- Baud rate: `9600`
- Parità: `None`
- Data bits: `8`
- Stop bits: `1`

Queste impostazioni sono quelle di default per l’interfaccia RS-232 del dispositivo.

## 4. Modalità di funzionamento
Il capitolo del manuale descrive l’uso dell’interfaccia seriale come segue:

- l’interfaccia RS-232 è pensata per connettere il modulo a un host come PC, POS o un microcontrollore;
- la comunicazione seriale deve usare parametri compatibili tra host e scanner;
- la velocità e il formato seriale devono essere selezionati in base ai requisiti del dispositivo host.

## 5. Connessione diretta a ESP32-S2 senza convertitore
Per un ESP32-S2 la connessione TTL è diretta e non richiede alcun convertitore di livello:

- `TTL232_TX` (pin 9) → RX UART dell’ESP32-S2
- `TTL232_RX` (pin 10) → TX UART dell’ESP32-S2
- `GND` comune
- `VCC 3.3V` solo se il modulo N1-W richiede alimentazione esterna dalla scheda

Non collegare i pin TTL direttamente a un’interfaccia RS232 tradizionale, ma l’ESP32-S2 può lavorare direttamente ai livelli TTL a 3.3 V.

## 6. Trigger e reset esterno
Nel connettore 13 pin sono presenti due segnali dedicati al trigger di scansione e al reset del modulo:

- Pin 2: `EXT_TRIG#` → ingresso trigger esterno
- Pin 3: `EXT_RST#` → ingresso reset esterno
- Pin 4: `EXT_DSF` → uscita LED "good read"
- Pin 5: `EXT_BUZ` → uscita beeper
- Pin 6: `EXT_LIGHT` → controllo illuminazione

### 6.1 Uso di EXT_TRIG# con ESP32-S2
L’`EXT_TRIG#` può essere usato per avviare una scansione in modalità hardware trigger.

- Il segnale ha un pull-up interno da 100k e può essere usato in modalità livello o impulso.
- In modalità livello, il decoder resta attivo finché il trigger rimane basso.
- In modalità impulso, un fronte di discesa con durata di almeno 50 ms avvia la scansione.
- Non abbassare `EXT_TRIG#` durante il power-on fino a quando il modulo non è avviato.

Con ESP32-S2, puoi collegare un GPIO a `EXT_TRIG#` in open-drain o a collettore aperto, in modo da evitare conflitti con il pull-up interno.
Assicurati che il livello logico sia compatibile con 3.3 V.

### 6.2 Uso di EXT_RST# con ESP32-S2
L’`EXT_RST#` serve per resettare il motore di scansione.

- Mantenere il pin basso per almeno 10 ms provoca il reset.
- Se non lo utilizzi, lascialo scollegato.
- In genere non è necessario usarlo se il modulo viene alimentato correttamente e non serve un reset hardware controllato dal micro.

Con ESP32-S2, puoi collegare un GPIO di reset a `EXT_RST#` e tirarlo basso quando vuoi un reset del modulo.
Questo può essere utile durante lo sviluppo o per ripristinare il modulo in caso di blocchi, ma non è obbligatorio per la comunicazione seriale TTL.

### 6.3 Necessità effettiva dei pin
Per una connessione diretta N1-W → ESP32-S2 via UART TTL, l’uso di `EXT_TRIG#` e `EXT_RST#` è opzionale:

- `EXT_TRIG#`: utile se desideri controllare la scansione via hardware indipendentemente dai comandi seriali.
- `EXT_RST#`: utile solo se vuoi avere un reset hard programmabile del motore.

Se il tuo software invia comandi seriali e il modulo si comporta correttamente, non è necessario usare né `EXT_TRIG#` né `EXT_RST#`.

## 7. Raccomandazioni pratiche
- Verifica sempre la massa comune tra scanner e host;
- Non collegare direttamente i pin TTL a un’interfaccia RS232 senza convertitore;
- Assicurati che la linea RX dell’host riceva il TX TTL dello scanner e viceversa;
- Usa un convertitore TTL/RS232 solo se il dispositivo host non supporta la seriale TTL diretta;
- Se il dispositivo è già impostato su USB CDC di default, disconnetti l’USB e usa solo il collegamento TTL secondo il progetto hardware.

## 8. Sintesi
Per usare l’N1-W con interfaccia RS232-TTL:

1. collega pin 9 (`TTL232_TX`) e pin 10 (`TTL232_RX`);
2. collega la massa comune;
3. usa i parametri seriali `9600 8N1`;
4. se serve RS232 reale, aggiungi un convertitore TTL<->RS232;
5. mantieni il livello 3.3 V su TTL232.

Questa impostazione garantisce compatibilità con la modalità seriale descritta nel manuale N1-W.

## 9. Verifica operativa RS232-TTL via comando query
Per verificare che la porta seriale tra ESP32-S2 e N1-W sia configurata correttamente, il firmware esegue un self-test in avvio (modalita seriale scanner).

Comandi di prova inviati su UART scanner:

- `SOH @QRYSYS EOT` (byte: `01 40 51 52 59 53 59 53 04`)
- `SOH QRYSYS EOT` (byte: `01 51 52 59 53 59 53 04`)

Dove:

- `SOH` = `0x01`
- `EOT` = `0x04`

Nei log sono evidenziati:

- `SCN TX ...` con dump esadecimale del frame inviato.
- `SCN RX ...` con dump esadecimale e ASCII della risposta ricevuta.

Se non arriva risposta, il log mostra `SCN RX ... no response`: in quel caso verificare cablaggio TX/RX incrociato, massa comune, modalita interfaccia scanner e baud `9600 8N1`.
