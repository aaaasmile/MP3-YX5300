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
monitor anziché nel loop().
2)Non riesco a creare una playlist continua come voglio. Se cambio il 
successivo file, esso viene fatto andare, ma poi finisce e bisogna premere
ancora il tasto next.
3) Il comando List non fa nulla se non lanciare play.




