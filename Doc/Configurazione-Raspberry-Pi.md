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

Occorre aver creato le credenziali per Google Drive e un foglio di calcolo Google Sheets ([vedi guida](Configurazione-Google-Drive-e-Google-Sheets.md)) per questo passo.

Si utilizzerà Python per accedere alle API di Google. Lanciare i seguenti comandi per installare le librerie ```gspread``` e ```oauth2client```:

```
sudo pip3 install gspread oauth2client
```

Lanciare il seguente comando per aggiornare la google lib:
```
sudo pip3 install --upgrade google-auth-oauthlib
```

Per interfacciarsi con il sensore bluetooth occorre installare la libreria [BlueST SDK](https://github.com/STMicroelectronics/BlueSTSDK_Python)

## Deploy

Copiare i file ```gateway.py``` e ```credentials.json``` in una cartella della scheda Raspberry Pi.

Posizionarsi all'interno della cartella e lanciare lo script utilizzando il comando:
```
python2.7 gateway.py
```