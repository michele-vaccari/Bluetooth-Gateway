# Configurazione Google Drive e Google Sheets

Di seguito vengono descritti i passi da effettuare per la configurazione dell'ambiente cloud di Google

## Setup

### Setup per l'accesso alle API di Google drive

* Creare un nuovo progetto accedendo alla [pagina di configurazione delle API di Google](https://console.developers.google.com/)
* Abilitare le API e i servizi sia per Google Drive che per Google Sheets
* Dal cruscotto delle API creare le credenziali
* Scaricare il file contenente le credenziali e rinominarlo in ```credentials.json```

### Setup per l'accesso ai fogli di calcolo di Google Drive

* Accedere al proprio account di Google Drive e creare un nuovo foglio di calcolo vuoto dal nome ```SensorData```
* Il foglio di calcolo ```SensorData``` dovrà contenre due fogli ```Temperature``` e ```Humidity```
* Aprire il file ```credentials.json``` con un editor di testo
* Condividere il foglio di calcolo ```SensorData``` con la mail contenuta nel campo ```client_email``` del file ```credentials.json```

Ora si può prosegure con la lettura della ([Configurazione Raspberry Pi 3 Model B+](Configurazione-Raspberry-Pi.md))