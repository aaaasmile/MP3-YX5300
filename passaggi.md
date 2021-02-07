# MP3PlayerYX
Questo è un player MP3 gestito dal microcontroller ESP8266 e dal modulo 
YX5300 UART TTL Serial Control MP3 Music Player Module.
La documentazione del YX5300 si trova su http://geekmatic.in.ua/pdf/Catalex_MP3_board.pdf

## Flash card
Uso una flash card micro da 16GB la quale va preparata prima di usarla.
Copiare i files dal pc semplicemente non funziona.
I nomi dei files sulla microcard devono essere nel fomato
001xxx.mp3. Sei lettere per il nome, le prime 3 sono l'indice.
Il folder, invece, ha due lettere tipo 01, 02 e così via.
1000 files per folder su 100 folders, per un totale max di 100 000 files
addressabili.

## Comunicazione
Il modulo usa una comunicazione seriale nel formato spiegato nella documentazione.
I pin addetti alla comunicazione seriale dell'ESP8266 sono D9 e D10, marcati come RX e TX.
Il collegamento con YX5300 va fatto cross. Completa il collegamnto del modulo con Vcc e Gnd
dell'ESP8266. In pratica, solo i primi 4 pin dell'ESP8266 sono collegati.

## Problemi
1)Non riesco a ricevere le risposte del player, sembrta che vengano mandate nel 
monitor anziché nel loop(). [DONE]
2)Non riesco a creare una playlist continua come voglio. Se cambio il 
successivo file, esso viene fatto andare, ma poi finisce e bisogna premere
ancora il tasto next.
3) Il comando List non fa nulla se non lanciare play.

4) La comunicazione seriale disponibile sull esp8266 sembra esseere limitata
ad una sola UART fullduplex, mentre sulla seconda solo in send.
L'interfaccia seriale di programmazione di platformio sembra  usare Rx2 e Tx2
sui pin D7 e D8. Ho provato a collegarli usando serial2, ma senza successo.
Anzi ho piantato il sistema. Quindi non riesco a leggere le risposte del modulo YX5300.
Non vedo come possa funzionare con HardwareSerial e Serial2.

5) Sulle query ricevo solo la risposta 0x41, ma non il valore della richiesta.

## Soluzione comunicazione seriale
Per farla andare in modo stabile, basta usare SoftwareSerial sui pin
D5 e D6 come nell'espio di Arduino Nano e il gioco è fatto.
Così si può avere l'interfaccia UART per la programmazione e il monitor,
mentre per l'Mp3 si usano i GPIO con emulazione software.
Il codice HardwareSerial lo rimuovo in quanto non può funzionare con Esp8266.

## Proposta
Quello che funziona al momento sembra sufficiente per avere una lista 
che viene eseguita in modo casuale, ma che rimane all'iterno del folder.
Si dovrebbe:
- Preparare la lista degli indici del folder in modo casuale
- Lanciare il primo elemento della lista
- Al termine dell'esecuzione, lanciare il prossimo elemento
Occorrerebbe un file che mappa il nome del file sulla scheda con il tag mp3.
Questo per mostrarlo in una lista sul webserver ed eseguirlo singolarmente.
In questo modo è anche possibile creare la lista random senza dover usare
la query tipo CMD_QUERY_TOT_TRACKS che chissà come funziona.
