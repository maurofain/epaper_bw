# Test Scanner da PC su ttyACM1

Questo test invia allo scanner la stessa sequenza query usata nel firmware ESP.
Per sicurezza, di default NON invia i comandi init interfaccia:
- `@SETUPE1\r`, `@INTERF0\r`, `@SETUPE0\r`

Questi comandi possono cambiare la modalita' interfaccia del modulo (es. RS232/CDC).
Usare `--with-init` solo quando strettamente necessario.

Sequenza standard (default):
- query: varianti `QRYSYS`
- start/stop scan: `0x01 'T' 0x04` e `0x01 'P' 0x04`

## Prerequisiti

- Scanner collegato al PC su `/dev/ttyACM1`
- Python 3
- Pacchetto `pyserial`

Installazione dipendenza:

```bash
pip install pyserial
```

## Esecuzione base

Dal root del progetto:

```bash
python3 scripts/scanner_pc_probe.py --port /dev/ttyACM1

Nota: in questa modalita' i comandi INIT_* sono saltati.
```

## Esecuzione con parametri espliciti

```bash
python3 scripts/scanner_pc_probe.py \
  --port /dev/ttyACM1 \
  --bauds 9600,115200,57600,38400,19200 \
  --attempts 3 \
  --response-timeout-ms 800 \
  --inter-cmd-ms 20 \
  --between-attempt-ms 100
```

## Esecuzione con comandi INIT (opzionale)

```bash
python3 scripts/scanner_pc_probe.py --port /dev/ttyACM1 --with-init
```

## Interpretazione output

- `TX ... hex=[...]`: comando trasmesso
- `RX ... hex=[...] ascii=[...]`: risposta ricevuta
- `RX ... no response`: nessun byte ricevuto entro timeout

Lo script termina con:
- codice `0` se riceve almeno una risposta
- codice `2` se non riceve risposte
- codice `1` per errore parametri/dipendenze
