# Configurazione Raspberry Pi 3 Model B+

Di seguito vengono descritti i passi da effettuare per la preparazione della scheda Raspberry Pi

## Setup

### Setup OS

* Scaricare l'immagine del sistema operativo [NOOBS](https://www.raspberrypi.org/downloads/noobs/)
* Scaricare il tool [SD Formatter](https://www.sdcard.org/downloads/formatter_4/index.html)
* Formattare la scheda SD utilizzando il tool SD Formatter
* Estrarre i file contenuti nell'archivio NOOBS all'interno della scheda SD
* Inserire la scheda SD ed alimentare la scheda Raspberry PI
* Seguire il wizard per installare e configurare Raspbian
* Aggiornare l'elenco dei pacchetti
	```
	sudo apt-get update
	```
* Aggiornare i pacchetti installati
	```
	sudo apt-get upgrade
	```

### Setup per l'accesso ai fogli di calcolo di Google Drive

Occorre aver creato le credenziali per Google Drive e Google Sheets ([vedi guida](Configurazione-Raspberry-Pi.md)) per questo passo.

Si utilizzerà Python per accedere alle API di Google. Lanciare i seguenti comandi per installare le librerie ```gspread``` e ```oauth2client```:

```
sudo pip3 install gspread oauth2client
```

Lanciare il seguente comando per aggiornare la google lib:
```
sudo pip3 install --upgrade google-auth-oauthlib
```

## Deploy

Copiare il contenuto della cartella ```RaspberryPi``` nel ```Desktop``` della scheda Raspberry Pi.

Copiare il file .json contente le credenziali delle API di Google all'interno della cartella ```Gateway``` e rinominarlo in ```credentials.json```

Lanciare lo script contenuto nella cartella Gateway utilizzando il comando:
```
python3 gateway.py
```