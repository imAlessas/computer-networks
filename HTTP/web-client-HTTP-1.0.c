
#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // write
#include <string.h>                 // strlen






int main(){

    // definizione di variabili locali
    struct sockaddr_in server_addr;     // struct per definire l'indrizzo del server
    int s;                              // socket
    int t;                              // variabile temporanea
    unsigned char * p;                  // puntatore per indirizzo IP
    int i;


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


    // richiesta al server
    char * request = "GET /\r\n";
    write(s, request, strlen(request));

    // leggo la risposta
    const int RESPONSE_SIZE = 2000000;      // dimensione del buffer
    char response[RESPONSE_SIZE];           // buffer per la risposta
    
    // sleep(2);
    for ( i = 0; t = read(s, response + i, RESPONSE_SIZE - 1 - i); i += t ) {}

    response[i] = 0;


    return 0;

} // main
