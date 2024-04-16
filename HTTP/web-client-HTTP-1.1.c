#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // write
#include <string.h>                 // strlen, strcmp
#include <stdlib.h>                 // atoi





// definisco il buffer che conterrà i campi dell'header così come arrivano, da cui leggiamo i caratteri
char hbuf[10000];

// è un array di 'coppie' (una mappa) che puntano ciascuna ad il nome di un campo e al suo valore nell'header presente in hbuf 
struct headers{
    char * n;   // nome
    char * v;   // valore
} h[100];       // definisco la tabella di indicizzazione



int main(){

    // definizione di variabili locali
    struct sockaddr_in server_addr;     // struct per definire l'indrizzo del server
    int s;                              // socket
    int t;                              // variabile temporanea
    unsigned char * p;                  // puntatore per indirizzo IP
    int i, j;
    char * statusline;


    // crea il socket
    s = socket( AF_INET, SOCK_STREAM, 0 );
    // printf("Socket: %d\n", s);

    // terminazione nel caso di errori
    if( s == -1){
        printf("ERRNO = %d (%d)\n", errno, EAFNOSUPPORT);
        perror("Socket fallita\n");
        return 1;
    }


    /* Setup per la request */

    // imposto i campi della struct sockaddr_in del server    
    server_addr.sin_family = AF_INET;              // la famiglia qualificherà il tipo specifico
    server_addr.sin_port = htons(80);              // la porta a cui ci vogliamo collegare
    // indirizzo IPv4 del server
    p = (unsigned char *) &server_addr.sin_addr.s_addr;
    p[0] = 142;     p[1] = 250;     p[2] = 187;      p[3] = 196;


    // connect al server
    t = connect(s, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));

    // terminazione nel caso di errori
    if(t == -1){ 
        perror("Connessione fallita\n");
        return 1;
    }


    /*
        Se si effettua una richiesta HTTP 1.1 con la stessa sintassi della 1.0, "GET / HTTP/1.1\r\n\r\n", notiamo che dopo aver consumato e stampato l'Header a video, il processo rimane in attesa e non termina l'esecuzione. Uno dei peccato di Berners Lee è stato considerare che la risposta finisse con la chiusura della connessione: non a caso il processo sta eseguendo la read bloccante. 
        In realtà è più conveniente usare la stessa connessione per fare più richieste. Questo perchè effettuare una connessione costa risorse. Inoltre il flusso dello stream parte lento e poi si stabilizza; aprire molte connessioni rende impossibile questa stabilizzazione rendendo ancora più inefficiente la gestione delle risorse.

        Per questo motivo, dall'HTTP 1.1, di default, non si chiude la connessione non si chiude dopo l'Entity-Body. Per fare ciò, è necessario inserire la stringa "Connection: close" alla fine della richiesta, tra un CRLF e l'altro [GET / HTTP/1.1\r\nConnection: close\r\n\r\n].
        Scegliere di chiudere la connessione è come fare un downgrade, per tutti i motivi già citati. È quindi necessario capire come gestire questo nuovo comportamento del protocollo.
        La possibilità di avere una connessione aperta ci permette fare più richiesta senza aprire/chiudere una connessione. Si potrebbe quindi implementare un loop di GET o di altre richieste sulla stessa connessione. Questa cosa con le precedenti versioni del protocollo HTTP non si potevano fare.
    */
    char * request = "GET / HTTP/1.1\r\n\r\n";
    write(s, request, strlen(request));



    /* Lettura della response - header */

    // inizializzo il primo puntatore di h[0].n al primo carattere del buffer dell'header (hbuf). Essendo la prima riga, questa è la status line
    statusline = h[0].n = hbuf;
    j = 0;

    // leggo un carattere alla volta dell'header
    for(i = 0; read(s, hbuf + i, 1); i++ ){

        // fine campo header
        if( hbuf[i - 1] == '\r' && hbuf[i] == '\n'){
            
            hbuf[i - 1] = 0;            // terminatore su \r
           
            if( !( h[j].n[0] ) )    // entro se sono alla fine dell'header
                break;
            
            h[++j].n = &hbuf[i + 1];    // imposto il nome della nuova riga della tabella
        }

        // fine nome campo header
        if( (hbuf[i] == ':') && (h[j].v == NULL) ){

            h[j].v = &hbuf[i + 1];      // imposto il valore della tabella
            hbuf[i] = 0;                // terminatore
        }
    }

    // stampo la tabella di indicizzazione, j contiene il numero di righe della tabella
    for(i = 0; i < j; i++)
        printf("%s —————> %s\n", h[i].n, h[i].v);
    printf("\n\n");




    /*
        È proprio durante questo ciclo che si blocca la read in quanto l'Entity-Body è stato letto ed inserito buffer response[] ma dato che la read() non sta ritornando 0 - indice del fatto che la connessone è stata chiusa - si è fermi nel ciclo.
            for ( i = 0; t = read(s, response + i, RESPONSE_SIZE - 1 - i); i += t ) {}
        Quando la connessione dopo un po' di tempo di inattività scade, la read() ritorna 0, il ciclo termina e il body viene printato. È quindi necessario modificare il ciclo for per gestire questa situazione.

        La soluzione non è del tutto banale. Nel caso in cui la richiesta abbia un errore, per esempio richiediamo una risorsa inesistente (come ppp.html), nell'header sarà presente un campo chiamato "Content-Length" che indica esattamente la lunghezza del file che verrà streamato. In questo caso l'idea è quella di trovare questo campo mentre stampiamo a video l'header, per chiarezza si sceglierà di creare un altro for loop ad-hoc per trovare il suddetto valore.
        Cosa succede però se la richiesta è ben formata e va a buon fine? Si sserva che poco prima dell'inizio documento HTML è presente un numero esadecimale (i.e. 493c). Si osserva inoltre che l'ultimo campo dell'Header ha valore "Transfer-Encoding —————>  chunked" che, come vedremo, suggerisce il tipo della codifica di trasferimento. Il valore esadecimale indica ESATTAMENTE il numero di byte contenuti all'interno dello stream che andremo a leggere.
    */

    int content_length;

    // estraggo la Content-Length dal h[i]
    for(i = 0; i < j; i++){
        /*
            Si utilizza la funzione strcmp() che ritorna 0 se le due stringhe sono uguali.
            Si utilizza poi la funzione atoi() per immagazzinare il valore trovato convertendolo in un intero
        */
        if( !strcmp( h[i].n , "Content-Length" ))
            content_length = atoi(h[i].v);
    }
    printf("content-length: %d\n", content_length);

    // preparo a leggere l'Entity-Body
    char response[2000000];           // buffer per la consumare il Body
    

    /*
        Al fine di supportare questo protocollo, si modifica il limite della read() a content_length che è esattamente la lunghezza in byte - ovvero il numero di caratteri - del file streammato.
    */
    for ( i = 0; t = read(s, response + i, content_length - i); i += t ) {}

    response[i] = 0;    // inserisco il terminatore alla fine del body

    printf("%s\n\n", response);




    return 0;

} // main
