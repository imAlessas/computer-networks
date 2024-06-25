#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // read, write, fork
#include <string.h>                 // strlen




// variabili globali
unsigned char envbuf[1000];
int pid, env_i, env_c;






// array di 'coppie' (una mappa) di puntatori che puntano ciascuna ad il nome di un campo e al suo valore nell'header presente in hbuf 
struct headers{
    char * n;   // nome
    char * v;   // valore
}


// definizione delle funzioni
void add_env(char *, char *);






int main(){

    // costanti
    const int PORT = 31415;                 // numero della porta
    const char CGI[100] = "/cgi-bin/";      // directory dove sono presenti le funzioni back-end 


    /* definizione di variabili locali */
    struct headers h[100];                  // definisco la tabella di indicizzazione
    struct sockaddr_in server_addr;         // struct per definire l'indrizzo del server
    struct sockaddr_in client_addr;         // struct per definire l'indrizzo del client remoto

    char hbuf[10000];                       // conterrà i campi dell'header così come arrivano
    char response[2000];                    // contiene la response
    char entity[1000];                      // contiene l'entity della response spezzettato
    char file_name[100];                    // contiene il file da che verrà eseguito come servizio di back-end

    int s, s_double;                        // socket
    char * command_line;                    // prima riga della request i.e.  GET /index.html HTTP/1.1
    char * method, * url, * version;        // token che voglio estrarre da command_line
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


        /* estraggo url nello stesso modo */
        url = command_line + i;   

        while( command_line[i] != ' ' ) i++;

        command_line[i++] = 0;


        /* estraggo anche version */
        version = command_line + i;     
        
        // cambio da ' ' a 0 per il fine riga (abbiamo messo il terminatore nel parsing precedente)
        while( command_line[i] != 0 ) i++;

        command_line[i++] = 0;

        printf("Method = %s\nURI = %s\nVersion = %s\n\n\n", method, url, version);



        /*
            In genere le funzioni back-end sono presenti in una cartella detta CGI oppure CGI-bin. Cerchiamo ora di implementare la gestione delle chiamate GET e POST. Si osserva che si cerca di implementarle nel livello più basso possibile: generalmente i web-server chiamano una shell anzichè il programma eseguibile, si è scelto così per ragioni di sicurezza anche se recentemente sono emersi degli attacchi che sfruttano determinate variabili di environment della shell stessa.
            Una volta controllato che la richiesta sia di un servizio di back-end (di conseguenza l'URL inizia con /cgi-bin/), salvo il perocorso del file/servizio richiesto; a questo punto controllo se il metodo utilizzato sia GET oppure POST. 
            Se il metodo è GET, scorro lungo il path (file_name) fino al punto interrogativo '?'; tutto ciò che segue è la QUERY_STRING. In questo ramo dell'if viene quindi confezionato l'environment necessario per la modalità GET, ovvero inserire dentro QUERY_STRING l'insieme dei parametri presenti nel form (presente in index.html)
            Se il metodo è POST, riempio la CONTENT_LENGTH utilizzando la variabile length, presa dagli header. Allo stesso modo, anche la CONTENT_TYPE, viene presa dagli header e inserita come environment.
            Se non è nessuno dei due metodi, notifico con il codice 501 - Not Implemented.
        */
        

        // controllo se viene richiesto qualcosa in cgi-bin
        if( !strncmp(CGI, url, strlen(CGI)) ){
            
            // imposto il file_name
            file_name = url + strlen(CGI);

            if( !strcmp(method, "GET") ){ // se il metodo è GET
                
                // scorro fino all'inizio della QUERY_STRING; delimitato da '?'
                i = 0;
                while( file_name[i] && file_name[i] != '?' )
                    i++;

                if( file_name[i] == '?' ){
                    
                    // null-termino
                    file_name[i] = 0;

                    // inserisco tutto ciò che segue '?' nella QUERY_STRING
                    add_env("QUERY_STRING", file_name + i + 1);
                }

                add_env("QUERY_STRING", "0");

            } else if ( !strcmp(method, "GET") ){ // se il metodo è POST

                char tmp_buf[10];
                
                sprintf(tmp_buf, "%d", length);
                
                add_env("CONTENT_LENGTH", tmp_buf);


            } else {    // notifico errore
                sprintf(response, "HTTP/1.1 501 NOT IMPLEMENTED\r\nConnection:close\r\n\r\n");

                // invio la response al client mediante la write()
                write(s_double, response, strlen(response));

                // chiudo il socket
                close(s_double);

                // evito creazione di processi zombie chiudendo il thread
                exit(1);
            }

        }



        // apro il file
        file = fopen(url + 1, "rw");

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
                            "</body></html>", url);

            // invio la response al client mediante la write()
            write(s_double, response, strlen(response));

            // chiudo il socket
            close(s_double);

            // evito creazione di processi zombie chiudendo il thread
            exit(1);
        }


        /*
            Nel momento in cui devo fare la response, mando la status_line e poi inizia l'interfacciamento con il programma.
        */

        // creo l'header della response (status_line) e lo mando con write()
        sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");
        write(s_double, response, strlen(response));





        // chiudo il file
        fclose(file);
    
        // chiudo il socket
        close(s_double);

        // evito creazione di processi zombie
        exit(1);
    }


    return 0;

} // main







void add_env(char * env_key, char * env_value){

    sprintf(envbuf + enc_c, "%s=%s", env_key, env_value);


} // add_env