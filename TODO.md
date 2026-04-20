# modifiche per nuova scheda EpaperQr


1. Dobbiamo convertire questo progetto pensato per una eps32s3 e un pannello epaper a 4 colori modificando le specifiche del pannello e del processore ed intgrando il nuovo hardware
2. Impostare il processore a ESP32-S2-MINI-2-N4
3. il pannello epaper è il Waveshare 1.54 pollici E-Paper Raw Display Panel V2 200 x 200 , che utilizza un pannello goodDisplay GDEY0154D67. I driver sono in  https://github.com/ZinggJM/GxEPD2
4. tabella Pin
| Pin | GPIO | Descrizione | 
|----|----|----|
| PIN 23 | USB- | USB-C pin DN|
| PIN 24 | USB+ | USB-C pin DP|
| PIN 28 | GPIO33 | EP BUSY |
| PIN 29 | GPIO34 | EP RESET |
| PIN 31 | GPIO35 | EP DC |
| PIN 32 | GPIO36 | EP CS | 
| PIN 33 | GPIO37 | EP SCK |
| PIN 34 | GPIO38 | EP SDI |
| PIN 38 | GPIO42 | QR BCRES |
| PIN 41 | GPIO45 | QE BCTRIG |
| PIN 45 | EN | RESET |
| PIN 4 | BOOT| |
| PIN 7 | GPIO3 | LED |
| PIN 21 | GPIO17 | TX |
| PIN 22 | GPIO18 | RX |
| PIN 39 | TXD0 | TXesp |
| PIN 40 | RXD0 | RXesp |
1. Il circuito ha 5 interfacce
   - TxD0 /RxD0 è l'interfaccia che va sul chip MAX232 e comunica conla MH1001
   - DN e DP Interfaccia USB per il flash del firmware
   - Interfaccia SPI+aux per il controllo Epaper (pin EP*)
   - TX / TX /BCRES / BCTRIG per connessione con scanner 
   - LED per il pilotaggio dei led WS2812B
2. Protocollo verso MH1001
   - la scheda inoltra i codice che riceve dallo scanner alla MH1001
3. Vanno predisposti 6 font 
   - Font solo numerici
     - GoogleSans100  da usarsi per visualizzare i numeri 0 - 9
     - GoogleSans60 da usarsi per visualizzare i numeri 10 - 99
   - Font alfanumerici
     - GoogleSans35 da usarsi per visualizzare i numeri 100 - 999 e testi fino a 3 caratteri
     - GoogleSans20 da usarsi per visualizzare testi fino a 6 caratteri 
     - GoogleSans15 da usarsi per visualizzare testi fino a 10 caratteri 
     - GoogleSans10 da usarsi per visualizzare testi fino a 15 caratteri 
4. i font da utilizzare saranno scelti in base alla stringa ricevuta considerando le caratteri numerici e alfanumerici presenti nella stringa e la lunghezza
5. - la scheda riceve via seriare RXesp un pacchetto in cui il primo byte è 0x00 se di tratta di un comando o 0x01 se si tratta di una stringa o 0x02 per il contrllo dei LED
6. Il ricevimento di un  stringa da visualizzare sul display nel formato  0x01, 0xLEN, Ch0, Ch1, ..., ChN - LEN = lunghezza della stringa seguita dai caratteri della stringa.E' ammesso un carattere CR per indicare la rottura della riga
7. Il ricevimento di un comando 0x00 0x00 corrisponde alla richiesta di refresh totale del display
8.  Il ricevimento di un comando 0x00 0x01 corrisponde alla richiesta di attivazione dello scanner richiesta di refresh totale del display
9.  Il comanda 0x02 seguito da 12 byte indica i valori da applicare ai 4 led WS2812b : l1r,l1g,l1b,l2r,l2g,l2b,l3r,l3g,l3b,l4r,l4g,l4b
10. la pubblicazione di una stringa va preceduta dalla cancellazione del precedente stringa con un refresh parziale
11. va calcolato il numero di righe necessarie contando le parole, organizzandole in righe tenedo conto del numero di caratteri per riga provando prima con il font più grande a diminuire di dimensione fino a trovare quella che permette la visualizzazione del testo intero. Le righe vanno composte raggruppando quante più parole possibili su una riga per poi passare alla riga successiva. Se il testo intero non è rappresentabile utilizando tutte le righe a disposizione verrà troncato lasciano gli ultimi 4 charatteri per indicare ' ...'
12. La gestione dello scanner newland N1-W prevede una fase di inizializzazione con l'invio di stringhe per il setup, dopo di che si rimane in attesa di letture che forniranno un codice che sarà ritrasmesso su TXesp
