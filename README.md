# Bluetooth Gateway

Analisi, progettazione e sviluppo di un gateway bluetooth e di un sensore di temperatura bluetooth.

Lo schema del sistema progettato è il seguente:

![System architecture diagram](Doc/Img/system-architecture-diagram.jpg)

* Il gateway si connette con il sensore di temperatura via bluetooth rimanendo in ascolto delle sue notifiche.
* Il sensore ogni dieci secondi invia via bluetooth la temperatura rilevata ai dispositivi connessi.
* Il gateway, connesso a internet tramite Wi-Fi, invia ogni notifica ricevuta in un foglio di calcolo di Google Drive.

Il progetto è stato realizzato per il corso di Sistemi di Elaborazione dell'Università degli Studi di Ferrara.

## Prerequisiti

Per la realizzazione del progetto è necessario disporre di:
* una connessione Internet
* un account Google
* una [Raspberry Pi 3 Model B+](https://www.raspberrypi.org/products/raspberry-pi-3-model-b-plus/)
* una [NUCLEO-L452RE](https://www.st.com/en/evaluation-tools/nucleo-l452re.html)
* una [X-NUCLEO-IDB05A1](https://www.st.com/en/ecosystems/x-nucleo-idb05a1.html)
* un [AM2320](https://cdn-learn.adafruit.com/downloads/pdf/adafruit-am2320-temperature-humidity-i2c-sensor.pdf)

## Come iniziare

Per configurare i vari componenti seguire le guide seguenti:
* [Configurazione NUCLEO-L452RE + X-NUCLEO-IDB05A1 + AM2320](Doc/Configurazione-NUCLEO.md)
* [Configurazione Raspberry Pi 3 Model B+](Doc/Configurazione-Raspberry-Pi.md)
* [Configurazione Google Drive e Google Sheets](Doc/Configurazione-Google-Drive-e-Google-Sheets.md)

## License

Questo progetto è concesso in licenza GNU General Public License - si veda il file [LICENSE](LICENSE) per i dettagli.