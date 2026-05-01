# Regole Progetto

## Log Avvio - Ultima Modifica

- Prima di suggerire o eseguire un flash, aggiornare sempre in `src/main.cpp` i campi di log avvio relativi all'ultima modifica:
  - `kAppLastChangeDescription`
  - `kAppLastChangeTimestamp` (o meccanismo equivalente)
- La proposta di flash e' ammessa solo dopo verifica che tali informazioni siano coerenti con la modifica appena fatta.
- il termine 'firmware' indica il programma per ESP32
- il termine 'tester' indica il programma python per il test