# Reti di Calcolatori

Questa repository contiene gli script prodotti durante il corso "Reti di Calcolatori" tenuto allUniversità degli Studi di Padova. La repo, come il corso, è divisa in due porzioni:

* La parte [_HTTP_](#http), si occupa di trattare tutto ciò che concerne dei livelli 5, 6 e 7 del modello ISO/OSI, in particolare si concentra sul protocollo `HTTP`

* La parte [_multimedia_](#multimedia), copre le tecniche di trasmissione a basso livello dei dati da un mittente a un destinatario

## HTTP

Nel modulo HTTP sono vengono scritti in `C` il [web-client](#web-client), il [web-server](#web-server) e il [web-proxy](#web-proxy), inoltre sono presenti alcune soluzioni alle prove di [esami passati](#esami).

### Web client

Sono fornite 3 implementazioni di un client web. Nella prima viene implementato un client mediante il protocollo [`HTTP 0.9`](./HTTP/web-client/HTTP-0.9/HTTP-0.9.c), ideato da _Berners Lee_.

La seconda implementazione racchiude un client che effettua le richiesta tramite [`HTTP 1.0`](./HTTP/web-client/HTTP-1.0/HTTP-1.0.c) e ne processa l'header.

Infine viene implementato anche un web client che supporta [`HTTP 1.1`](./HTTP/web-client/HTTP-1.1/HTTP-1.1.c) e quindi è in grado di consumare il body tramite i _chunk_.

### Web server

Viene anche fornita l'implementazione di un web [server basico](./HTTP/web-server/basic/basic.c) che si occupa solo di restittuire pagine lette e invece un [server gateway](./HTTP/web-server/gateway/gateway.c) che consente anche di eseguire altri script.

### Web proxy

È anche presente l'implementazione di un [web-proxy](./HTTP/web-proxy/web-proxy.c) che gestisce sia la richiesta in chiaro `GET` che la  `CONNECT`.

### Esami

Nella directory [`HTTP/exams/`](./HTTP/exams/) è possibile trovare diverse prove degli esami passati, di seguito elencati.

* [Server `content-length`](./HTTP/exams/2014-06-26/guidelines.md)

* [Server `reflect`](./HTTP/exams/2015-07-03/guidelines.md)

* [Client `cache`](./HTTP/exams/2015-07-24/guidelines.md)

* [Server `auth`](./HTTP/exams/2018-06-20/guidelines.md)

* [Server `auth`](./HTTP/exams/2018-06-20/guidelines.md)

* [Server `blacklist`](./HTTP/exams/2019-02-01/guidelines.md)

* [Server `cookie`](./HTTP/exams/2020-09-03/guidelines.md)

* [Server `ETag`](./HTTP/exams/2023-08-29/guidelines.pdf)

* [Server `chunked`](./HTTP/exams/2024-02-24/guidelines.pdf)

Inoltre il file [`HTTP/doc/main.pdf`](./HTTP/doc/main.pdf) contienel gli scirpt più importanti che possono essere portati all'esame come supporto alla prova di programmazione. Si consiglia inoltre di consulatre la [lista di repo](https://github.com/stars/imAlessas/lists/rdc-unipd) che contiene altre repository ben fornite.

### Esercizi

In [`HTTP/exercies/`](./HTTP/exercies/) sono presenti anche due semplici esercizi che implementano la conversione [host-to-network](./HTTP/exercies/host-to-network/hton.c) e la codifica [base64](./HTTP/exercies/base64/base64.c).

## Multimedia

All'interno del modulo multimedia sono presenti gli script `.py` e i report `.tex` dei due homework richiesti durante il corso.

### Homework 1

Il primo homework riguarda la __codifica lossless__ di immagini, in particolare si occupa esplorare due tipi di codifiche implementate attraverso due script. La [codifica semplice](./multimedia/hw-1/script/simple_coding.py) si basa su un semplice predittore del pixel successivo mentre la [codifica avanzata](./multimedia/hw-1/script/advanced_coding.py) si basa su un modello predittivo più complesso, focolazzandosi su più pixel attorno a quello da predire. Lo studio e l'analisi delle prestazioni di tali codifiche sono descritte nel [report](./multimedia/hw-1/main.pdf).

### Homework 2

Il secondo homework studia la __latenza di rete__: si è implementato uno [script](./multimedia/hw-2/script/script.py) che analizza le prestazioni di rete inviando pacchetti ad un determinato server. In particolare viene studiato il _numero di link_, il Round Trip Time (_RTT_) per poi calcolare il __throughput__. Il [report](./multimedia/hw-2/main.pdf) contiene tutto ciò che è necessario sapere per poter eseguire il codice e discute i risultati ottenuti.
