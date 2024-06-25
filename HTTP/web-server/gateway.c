#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // read, write, fork
#include <string.h>                 // strlen





struct sockaddr_in server_addr;     // struct per definire l'indrizzo del server
struct sockaddr_in client_addr;     // struct per definire l'indrizzo del client remoto



char hbuf[10000];           // conterrà i campi dell'header così come arrivano
char response[2000];        // contiene la response
char entity[1000];          // contiene l'entity della response spezzettato
char cmd[100];              // contiene il comando da eseguire


// array di 'coppie' (una mappa) che puntano ciascuna ad il nome di un campo e al suo valore nell'header presente in hbuf 
struct headers{
    char * n;   // nome
    char * v;   // valore
} h[100];       // definisco la tabella di indicizzazione




int main(){

    const int PORT = 31415;                 // numero della porta
    const char EXEC[100] = "/exec/";        // directory dove verranno eseguiti i comandi             


    // definizione di variabili locali
    int s, s_double;                        // socket
    char * command_line;                    // prima riga della request i.e.  GET /index.html HTTP/1.1
    char * method, * file_name, * version;  // token che voglio estrarre da command_line
    FILE * file;                            // file richiesto dal Client
    int i, j;                               // indici multifunzione
    int t;                                  // variabile temporanea


    // crea il socket
    s = socket( AF_INET, SOCK_STREAM, 0 );

    // terminazione nel caso di errori
    if(s == -1){
        perror("socket() failed");
        printf("errno: %d\n", errno);
        return 1;
    }

    // imposto indirizzo del server
    server_addr.sin_family      = AF_INET;          // famiglia di indirizzo, specializza sockaddr → sockaddr_in
    server_addr.sin_port        = htons(PORT);     // porta a cui è associato il socket
    server_addr.sin_addr.s_addr = 0;                // indirizzo a cui è associato il socket



    // lego socket s a server_addr tramite bind()
    t = bind(s, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));    

    // terminazione nel caso di errori
    if(t == -1){
        perror("bind() failed");
        printf("errno: %d\n", errno);
        return 1;
    }


    // metto in listen s con dimensione della coda = backlog = 5
    t = listen(s, 5);

    // terminazione nel caso di errori
    if(t == -1){
        perror("listen() failed");
        printf("errno: %d\n", errno);
        return 1;
    }




    int len = sizeof(struct sockaddr);
    
    while(1){ // parte il server e non muore mai
        
        close(s_double);

        // accetto la prima richiesta di connessione della coda
        s_double = accept(s, (struct sockaddr *) &client_addr, &len);

        // se siamo nel processo padre (fork != 0) ritorniamo su accept()
        if(fork()) continue;


        /* - - da qui in poi ci sono solo processi figli - - */

        // terminazione nel caso di errori
        if(s_double == -1){
            perror("accept() failed");
            printf("errno: %d\n", errno);
            return 1;
        }



        // consumo la richiesta in maniera conforme alla grammatica
        // codice molto simile al consume della response web-client/HTTP-1.1.c

        command_line = h[0].n = hbuf;
        j = 0;

        // leggo un carattere alla volta dell'header
        for(i = 0; read(s_double, hbuf + i, 1); i++){

            // fine campo header
            if( hbuf[i - 1] == '\r' && hbuf[i] == '\n' ){
                
                hbuf[i - 1] = 0;            // terminatore su \r
            
                if( !( h[j].n[0] ) )        // entro se sono alla fine dell'header
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
        printf("\n");



        /* tokenizzazione della request */

        // inizializzo metodo che punta al'inizio della command_line
        method = command_line;      // = command_line[0]
        
        // inizializzo i
        i = 0;

        // mi fermo quando trovo uno spazio 
        while( command_line[i] != ' ' ) i++;

        // null-termino method
        command_line[i++] = 0;


        /* estraggo file_name nello stesso modo */
        file_name = command_line + i;   

        while( command_line[i] != ' ' ) i++;

        command_line[i++] = 0;


        /* estraggo anche version */
        version = command_line + i;     
        
        // cambio da ' ' a 0 per il fine riga (abbiamo messo il terminatore nel parsing precedente)
        while( command_line[i] != 0 ) i++;

        command_line[i++] = 0;

        printf("Method = %s\nURI = %s\nVersion = %s\n\n\n", method, file_name, version);



        /*
            Vogliamo che il nostro web server, oltre che a restituire files, sia in grado anche di eseguire un programma e restituirne l'output, che deve essere visibile dal cliente: in questo modo si interfaccia il web ad un'applicazione.
            Come fa un web server a capire se una richiesta, anzichè essere il nome di un file, è un URI/URL che fa riferimento ad un servizio? Decidiamo che se il file richiesto, ovvero il file_name, inizia per '/exec' sappiamo che non è un file ma in realtà è un eseguibile.
            Se il paragone è vero, il file lo devo quindi utilizzare in modo diverso. Ipotizziamo che ci si voglia interfacciare al sito come se fosse una shell. In altre parole si vuole che se si digita '/exec/ls' in realtà si voglia eseguire a shell il comand 'ls'. Si utilizza la libreria system che permette proprio di fare ciò. Si osserva che il risultato del comando viene direzionato su un file il quale verrà mostrato di seguito.
        */
        
        // controllo inizio di file_name
        if( !strncmp(EXEC, file_name, strlen(EXEC)) ){

                // inizializzo l'inizio del comando
                sprintf(cmd, "%s > output.html", file_name + strlen(EXEC));

                // chiamo il comando
                system(cmd);

                // aggiorno il file_name in modo tale da mostrare il risultato
                strcpy(file_name, "/output.html");
        }


        // apro il file
        file = fopen(file_name + 1, "rw");

        // se il file non esiste
        if(file == NULL){
            // creo messaggio di errore
            sprintf(response, "HTTP/1.1 404 NOT FOUND\r\nConnection: close\r\n\r\n"
                            "<html><head><title>404 Not Found</title><style>"
                            "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }"
                            "h1 { font-size: 50px; color: #ff0000; }"
                            "p { font-size: 20px; color: #333; }"
                            "</style></head><body>"
                            "<h1>404 Not Found</h1>"
                            "<p>Sorry, the file <strong>%s</strong> was not found on this server.</p>"
                            "</body></html>", file_name);

            // invio la response al client mediante la write()
            write(s_double, response, strlen(response));

            // chiudo il socket
            close(s_double);

            // evito creazione di processi zombie chiudendo il thread
            exit(1);
        }


        /* il file esiste e va restituito */

        // creo l'header della response e lo mando con write()
        sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");
        write(s_double, response, strlen(response));

        // finchè non finisce il file
        while( !feof(file) ) {
            // leggo il file
            fread(
                entity,      // buffer dove inserire le informazioni lette
                1,           // dimensione di informazione prelevata = 1 byte
                1000,        // numero di informazioni per lettura, 1000 * 1 = 1000 bytes
                file         // da dove leggere informazioni
            );

            // mando entity-body spezzettato
            write(s_double, entity, strlen(entity));
        }

        // chiudo il file
        fclose(file);
    
        // chiudo il socket
        close(s_double);

        // evito creazione di processi zombie
        exit(1);
    }


    return 0;

} // main

