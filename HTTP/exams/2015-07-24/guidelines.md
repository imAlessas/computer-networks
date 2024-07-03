# Esame di Reti di Calcolatori

> _24 Luglio 2015_

Si modifichi `web-client.c` in modo che incorpori un meccanismo di __caching__ delle risorse scaricate, facendo riferimento ai seguenti punti:

1. Si utilizzi l'header `Last-Modified` dell'HTTP/1.0 documentato alla sezione 10.10 della RFC 1945;

1. Ad ogni risorsa scaricata si associ un file:
    - il cui _nome_ corrisponde all'URI della risorsa (nel quale il carattere '/' viene sostuito dal carattere '_');
    - il cui _contenuto_ è composta da;
        - una prima riga contente la data di download della risorsa (espressa nel modo più conveniente)
        - il contenuto della risorsa (entity body);
    - la cui _cartella_ di salvataggio è `./cache/`, figlia del working directory del programma `web-client.c`,

1. Per la gestione della data:
    - si faccia riferimento al formato __http-date__ (cfr. RFC 1945 Sezione 3.3);
    - si utilizzino le funzioni, documentate nel manuale UNIX nelle apposite sezioni riportate tra parentesi;
        - `time(2)` per ottenere la data espressa ini _secondi_ a partire dal 1/1/1970 (epoch) nel tipo int rinominato time_t;
        - `localtime(3)` per _scomporre_ la data espressa in "epoch" nelle sue componenti (ora, minuti, ...) riportate ciascuna in un campo della struttura struct tm e viceversa;
        - `mktime(3)` per effettuare l'operazione inversa;
        - opzionalmente utilizzare `strftime(3)` per formattare (analogamente alla printf) le componenti della data presenti nei campi della struct tm in una stringa e `strptime(3)` per effettuare (similmente alla scanf) l'operazione inversa.
