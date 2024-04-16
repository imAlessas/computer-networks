#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // write
#include <string.h>                 // strlen, strcmp





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


    // richiesta HTTP 1.0 al server
    char * request = "GET / HTTP/1.0\r\n\r\n";
    write(s, request, strlen(request));



    /* Lettura della response - header */

    // inizializzo il primo puntatore di h[0].n al primo carattere del buffer dell'header (hbuf). Essendo la prima riga, questa è la status line
    statusline = h[0].n = &hbuf;
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




    /* Lettura della response - Body*/

    // preparo a leggere l'Entity-Body
    const int RESPONSE_SIZE = 2000000;      // dimensione del buffer
    char response[RESPONSE_SIZE];           // buffer per la consumare il Body
    
    // leggo l'Entity-Body
    for ( i = 0; t = read(s, response + i, RESPONSE_SIZE - 1 - i); i += t ) {}

    response[i] = 0;    // inserisco il terminatore alla fine del body

    print("%s\n\n", response);




    return 0;

} // main
