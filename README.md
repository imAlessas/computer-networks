# Reti di Calcolatori

Questa repository contiene gli script prodotti durante il corso "Reti di Calcolatori" tenuto allUniversità degli Studi di Padova. La repo, come il corso, è divisa in due porzioni:

* La parte __HTTP__, si occupa di trattare tutto ciò che concerne dei livelli 5, 6 e 7 del modello ISO/OSI, in particolare si concentra sul protocollo ```HTTP```

* La parte __multimedia__, copre le tecniche di trasmissione a basso livello dei dati da un mittente a un destinatario

## Multimedia

All'interno del modulo multimedia sono presenti gli script ``.py`` e i report ```.tex``` dei due homework richiesti durante il corso.

### Homework 1

Il primo homework riguarda la __codifica lossless__ di immagini, in particolare si occupa esplorare due tipi di codifiche implementate attraverso due script. La [codifica semplice](./multimedia/hw-1/script/simple_coding.py) si basa su un semplice predittore del pixel successivo mentre la [codifica avanzata](./multimedia/hw-1/script/advanced_coding.py) si basa su un modello predittivo più complesso, focolazzandosi su più pixel attorno a quello da predire. Lo studio e l'analisi delle prestazioni di tali codifiche sono descritte nel [report](./multimedia/hw-1/main.pdf).

### Homework 2

Il secondo homework studia la __latenza di rete__: si è implementato uno [script](./multimedia/hw-2/script/script.py) che analizza le prestazioni di rete inviando pacchetti ad un determinato server. In particolare viene studiato il _numero di link_, il Round Trip Time (_RTT_) per poi calcolare il __throughput__. Il [report](./multimedia/hw-2/main.pdf) contiene tutto ciò che è necessario sapere per poter eseguire il codice e discute i risultati ottenuti.
