# Esame di Reti di Calcolatori

> _3 Luglio 2015_

Si modifichi il programma che implementa il web server in modo che questo, non appena riceve dal client una request per la risorsa corrispondente al path `/reflect`, anziché cercare un file da aprire ed inviare, invii al client una response nella quale l’entity body sia:

1. Il testo esatto corrispondente all’__intera request__ inviata dal client al server, comprensiva di tutti gli elementi che la compongono;

1. `CRLF`;

1. L’__indirizzo IP__ in notazione decimale puntata da cui il client ha inviato la propria request;

1. `CRLF`;

1. Il __port__ da cui il client ha effettuato la propria request.
