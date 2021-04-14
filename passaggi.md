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
Uso ESP8266 con i pin 
D5 per RX  ------> TX di YX5300
D6 per TX  <-----  RX di YX5300


## Problemi
1)Non riesco a ricevere le risposte del player, sembrta che vengano mandate nel 
monitor anziché nel loop(). [DONE]
2)Non riesco a creare una playlist continua come voglio. Se cambio il 
successivo file, esso viene fatto andare, ma poi finisce e bisogna premere
ancora il tasto next. [DONE]
3) Il comando List non fa nulla se non lanciare play. Da rimuovere. [DONE]

4) La comunicazione seriale disponibile sull esp8266 sembra esseere limitata
ad una sola UART fullduplex, mentre sulla seconda solo in send.
L'interfaccia seriale di programmazione di platformio sembra  usare Rx2 e Tx2
sui pin D7 e D8. Ho provato a collegarli usando serial2, ma senza successo.
Anzi ho piantato il sistema. Quindi non riesco a leggere le risposte del modulo YX5300.
Non vedo come possa funzionare con HardwareSerial e Serial2.
Risolto collegando i pin D5 e D6 e usato SoftwareSerial. [DONE]

5) Sulle query ricevo solo la risposta 0x41, ma non il valore della richiesta.
Le risposte alle richieste CMD_QUERY_FLDR_TRACKS, CMD_QUERY_TOT_TRACKS e 
CMD_QUERY_FLDR_COUNT mandano sempre la risposta 0x41. 
La riposta "Completed play num xy" viene mandata due volte consecutive.
La prima quando la canzone finisce, la seconda in risposta al comando CMD_PLAY_FOLDER_FILE.
Il problema l'ho risolto concentrandomi sui comandi che funzionano in modo
affidabile, vale a dire: CMD_PLAY_FOLDER_FILE, CMD_SHUFFLE_PLAY, CMD_VOLUME_UP, CMD_VOLUME_DOWN e CMD_SEL_DEV. [DONE]

## Modalità di esecuzione
Utilizzo MP3PlayerYX in automobile collegato al Line-In della radio, la stessa funzione dell'ipod.
Per eventuali comandi uso il browser dello smartphone, in quanto MP3PlayerYX offre un accespoint wireless.

Con un disposivo che possiede una marea di files mp3, se ad ogni reset mi lancia la 
stessa canzone vado in paranoia.   
Un problema che ho con l'ipod collegato alla porta USB della radio è che la sequenza 
non è casuale. La radio dell'automobile mi lancia inizialmente una sequenza random, che però si 
ripete allo stesso modo all'avvio successivo dell'auto, per esempio dopo una pausa all'autogrill. 

Se invece l'ipod è collegato alla Line-In come MP3PlayerYX, la sequenza random è si casuale,
ma la sequenza random dell'ipod mi ripete i files prima di averli lanciati tutti.

All'avvio di MP3PlayerYX viene creata una sequenza casuale dei files mp3 presenti nel folder 1.
Nessuna ripetizione del file prima di aver completato tutto il folder e play a ciclo continuo. 

La casualià la raggiungo utilizzando la lettura del pin analogico A0 combinata con il tick in microsecondi
della prima risposta sulla porta seriale del player mp3. Mi aspetto almeno 500 microsecondi di indeterminazione su questo valore, il quale mi serve per inizializzare il random seed.

Lo svantaggio principale è che non è possibile comandare MP3PlayerYX con i tasti sul volante
come avviene con l'Ipod attaccato alla porta USB. Da vedere se sia possibile comandare il browser
dell'Iphone con Siri per azionare i 4 tasti per i comandi: next song, prev song, next folder e prev folder.

## Comunicazione seriale usata
Per farla andare in modo stabile, basta usare SoftwareSerial sui pin
D5 e D6 come nell'espio di Arduino Nano e il gioco è fatto.
Così si può avere l'interfaccia UART per la programmazione e il monitor,
mentre per l'Mp3 si usano i GPIO con emulazione software.
Il codice HardwareSerial lo rimuovo in quanto non può funzionare con Esp8266.

## Ulteriori sviluppi
Quello che funziona al momento sembra sufficiente per avere una lista 
che viene eseguita in modo casuale, ma che rimane all'iterno del folder.
Si dovrebbe:
- Preparare la lista degli indici del folder in modo casuale [DONE]
- Lanciare il primo elemento della lista [DONE]
- Al termine dell'esecuzione, lanciare il prossimo elemento [DONE]

- Occorrerebbe un file che mappa il nome del file sulla scheda con il tag mp3.
Questo per mostrarlo in una lista sul webserver ed eseguirlo singolarmente.
In questo modo è anche possibile creare la lista random senza dover usare
la query tipo CMD_QUERY_TOT_TRACKS che chissà come funziona.
- La sincronizzazione della flash card va fatta dal portatile con un programma apposta.
Non deve essere itunes, ma occorre una repository con i files che abbiano lo stesso nome che sulla carta
che semplicemente vengono tenuti sincronizzati, dove i files sul portatile fanno da master.
Bisogna anche tenere il collegamnto tra il file mp3 originale e completo con il nome del file
cambiato apposta per essere usato su MP3PlayerYX.
Alla fine del processo, un file per ogni folder dovrebbe essere generato e messo sull'ESP8266.
Questo file viene usato per generare la sequenza casuale di play.
