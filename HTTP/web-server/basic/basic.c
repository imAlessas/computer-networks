#include <stdio.h>  // printf, perror, fopen, fread, feof, fclose
#include <string.h> // strlen
#include <stdlib.h> // exit
#include <stdlib.h> // errno
#include <unistd.h> // read, write, fork
#include <sys/socket.h> // socket, bind, listen, accept
#include <arpa/inet.h>  // htons, sockaddr, sockaddr_in





struct sockaddr_in server_addr;     // struct per definire l'indrizzo del server
struct sockaddr_in client_addr;     // struct per definire l'indrizzo del client remoto



char hbuf[10000];           // conterrà i campi dell'header così come arrivano
char response[2000];        // contiene la response
char entity[1000];          // contiene l'entity della response spezzettato


// array di 'coppie' (una mappa) che puntano ciascuna ad il nome di un campo e al suo valore nell'header presente in hbuf 
struct headers{
    char * n;   // nome
    char * v;   // valore
} h[100];       // definisco la tabella di indicizzazione




int main(){

    const int PORT = 31415;                 // numero della porta

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


    /*
        È necessario che questo socket venga associato ad indirizzo e un port. L'indirizzo è l'indirizzo di rete della macchina, di conseguenza non è necessario indicarlo. In secondo luogo va impostato il port in quanto deve avere un valore NOTO in quanto il server deve essere raggiunto. Proprio per questo motivo non è possibile chiedere al sistema di utilizzare un port libero a caso, ma è necessario fare il binding manuale ad un port noto.
        Deve essere quindi definita una sockaddr_in che è l'indrizzo del server, definendo i seguenti campi:
            → 'sin_family', che è quella che specializza la sockaddr nella sockaddr_in ed ha il valore di AF_INET
            → 'sin_port', che indica il port; si dovrebbe mettere 80, ma dal momento che questo programma viene eseguito in un server web, è necessario scegliere un altro port, come 31415; ricordare di fare un htons() del valore
            → 'sin_addr.s_addr' indica l'indirizzo a cui è associato il socket, se impostato a zero il socket sarà visibile sul port 31415, qualunque sia l'indirizzo
    */
   
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = 0;

    /*
        Una connessione è univocamente definita da 4 valori: indirizzo sorgente, port sorgente, indirizzo destinazione, port destinazione. Proprio per questo motivo, un port, all'interno della stessa macchina, può essere associato a più indirizzi.
        Il singolo port può essere utilizzato per più connessioni perchè molti potranno collegarsi al port. Non si devono però mischiare connessioni da altre connessioni: ecco che è necessario invece dei SOCKET DIVERSI. 
            • Il socket è un numero univoco definito all'interno dello spazio del PROCESSO; abbiamo visto che il socket è un numero identificatico che si sovrappone ai file descriptor di un processo e indica un endpoint di un canale di connessione su cui è possibile effettuare operazioni di lettura e scrittura. 
            • Il port è un numero univoco all'interno del SISTEMA; associato il socket al port 31415, non ci può essere nessun altro socket all'interno del sistema che si associa al port 31415. Poichè ci possono essere tanti client che si collegano al port 31415, ci saranno all'interno del processo più socket associati alla porta 31415.

        Attraverso la chiamata a sistema bind(), che in inglese significa legare, che associa il socket ad un indirizzo port. I parametri della bind sono gli stessi della chiamata connect() effettutata nel web-client, ma la funzione è completamente diversa. 
            → La connect() infatti richiede un socket e una struct sock_addr, che rappresenta l'indirizzo REMOTO a cui il client vuole connettersi. L'indirizzo e il port locale sono sottointesi e possono completamente essere ignorati.
            → Nella bind() invece non ci si collega a nessuno: fornito il socket lo si lega all'indirizzo fornito. In questo modo, il processo che effettua la bind() sarà l'unico nel sistema che potrà avere associato il port. In questo modo, nello stesso processo si possono aprire tante connessioni allo stesso indirizzo (e quindi stesso port) mediante l'utilizzo di socket diversi.
    */

    // si passa il puntatore a server_addr castato come sockaddr in modo da gestire il problema del polimorfismo con i puntatori
    t = bind(s, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));    // lego socket s a server_addr

    // terminazione nel caso di errori
    if(t == -1){
        perror("bind() failed");
        printf("errno: %d\n", errno);
        return 1;
    }


    /*
        Adesso il nostro socket deve essere predisposto all'ascolto, ovvero a ricevere le richieste di connessione tramite la chiamata a sistema listen(). Una volta chiamata la listen() sul socket, questo non permette più connessioni attive ma è fatto solo ed esclusivamente per connettersi passivamente. 
        La chiamata listen() marchia il socket come un socket a cui possono arrivare richieste di connessione, detto anche socket passivo. Oltre al socket, come parametro richiede anche un numero, il 'backlog', che è la massima lunghezza a cui la coda delle connessioni pendenti può crescere.
        Il fatto che questa sarà una connessione passiva, indicando che non saremo noi a decidere quando aprire una connessione, rende più critica la gestione. Da un punto di vista puramente statistico si è osservato che avendo un numero di eventi casuali che accadono e distribuendo le distenza temporali tra di loro, si ottiene una distribuzione esponenziale. Questo significa che gli eventi non sono sparsi in maniera uniforme, tantomeno Gaussiana, ma occorrono molto vicini tra loro [teoria delle code]. Lo stesso risultato statistico lo si può osservare quando di notte e le strade sono libere, in un rotonda ci troviamo una macchina singola che passa: due eventi causuali che avvengon praticamente allo stesso tempo.
        Statisticamente quindi, in un intervallo temporale di richieste, ci saranno molte richieste che sono temporalmente vicine tra loro e poche richieste lontane: proprio per questo motivo è necessario accodare le richieste. Se nel momento della connessione, la coda è piena, la connessione non viene accettata.
    */

    // metto in listen s con  dimensione del backlog = 5
    t = listen(s, 5);

    // terminazione nel caso di errori
    if(t == -1){
        perror("listen() failed");
        printf("errno: %d\n", errno);
        return 1;
    }


    /*
        Le richieste di connessione possono quindi rimanere pendenti fino a che noi non le accettiamo. Di conseguenza, la funzione accept() può essere visto come lo scodamento delle connessioni della coda di listen().
        L'accept() chiede come parametro il socket, una struct sockaddr (che contiene indirizzo e port del socket remoto) e il puntatore alla lunghezza del sockaddr, che verrà riempito durante la funzione. La chiamata accept() ritorna un numero intero, che è molto più di un messaggio di errore, restituisce un NUOVO SOCKET. È un socket che non abbiamo create con la chiamata a sistema socket(): è un duplicato del socket s che è collegato anch'esso al port 31415 ma non è passivo (in ascolto) bensì è attivo, quindi NON in stato di listen.
        Nel momento in cui noi facciamo l'accept(), noi usiamo il socket in ascolto il quale non si collegherà MAI; appena arriva una richista cliente, viene passata ad un socket vero. Di conseguenza, un socket s vedrà moltissime connessioni ma non parteciperà mai ad una connessione perchè non verrà mai usato per fare delle read() o delle write().
        Nascono quindi tanti socket, tutti associato allo stesso port, e connessi ciascuno ad una connessione diversa: nel momento in cui facciamo una accept() un socket nuovo viene creato, connesso e su quello si effettuano le operazioni di read() e write().

        Il server deve sempre essere pronto a soddisfare le richieste di connessione e a fornire dati: non deve mai terminare, non deve mai morire. Ecco perchè l'accept() sarà dentro ad un loop infinito. In secondo luogo si osserva che in realtà l'accept(), dopo aver ricevuto la connessione, deve creare un processo, o un thread, che si occupa della connessione e quindi deve tornare subito ad accettare quella dopo.
        A questo proposito utilizziamo la chiamata a sistema fork(). Questa chiamata ha l'obiettivo di far continuare il flusso del programma, non solo dal processo da cui esista ma anche da un altro processo. La funzione fork() è l'unico modo che si ha (in UNIX) per creare dei processi: il processo chiama il fork() e questa chiamata genera un altro processo figlio, identico al padre. L'unica differenza che distingue i due processi è il valore di ritorno di fork(): il figlio come valore di ritorno ha 0. Attraverso la chiamata a sistema exec(), il figlio carica il nuovo programma da eseguire, diventando quindi diverso dal padre.
        Inserendo un controllo di ritorno sulla fork(), il padre e il figlio hanno lo stesso codice, ma grazie al controllo seguono un flusso diverso.
    */

    int len = sizeof(struct sockaddr);

    while(1){
        
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


        /*
            Parsando la request si pone l'attenzione su alcuni campi interessanti.
            • Il campo 'User-Agent' specifica con quale browser si è collegati al server per poter eventualmente adattare il contenuto al tipo di browser: questo è alla base della responsiveness dei sistemi.
            • Il campo 'Referer' è ancora più importante. Se in una pagina web viene premuto un link ad un'altra pagina, il protocollo HTTP richiede di specificare da quale pagina si arriva. Questo meccanismo è fondamentale per l'aspetto economico: l'interesse di pubblicare pagine web è stato dovuto a ragioni principalmente commerciali. Di conseguenza capire come ha fatto un Client a trovare la mia pagina è fondamentale: in un motore di ricerca, quando una query da cercare, non tutti i risultati appaiono in quanto soddisfano meglio la richiesta effettuata ma perchè qualcuno ha pagato per apparire (advertisement). Il referer è ciò che garantisce che davvero chi è arrivato alla mia pagina è arrivato grazie ad un determinato motore di ricerca oppure una pagina di riferimento. Questo header dal punto di vista tecnico non serve assulutamente a niente, ha però un valore immenso.
            • L'header 'Accept' elenca i tipi di file che il browser è in rado di elaborare e fa parte del livello di 'Presentation'. È una informazione importantissima, basti pensare al fatto che un Client elenchi tutti i formati in grado di gestire; l'entity body, di conseguenza, non è più un file, ma diventa un oggetto con un tipo.
            • Il campo 'Accept-Language', che fa parte anch'esso del Presentation layer, contiene una lista con priorità delle lingue preferite dal Client.
        */

        // stampo la tabella di indicizzazione, j contiene il numero di righe della tabella
        for(i = 0; i < j; i++)
            printf("%s —————> %s\n", h[i].n, h[i].v);
        printf("\n");



        /*
            È necessario ora tokenizzare la richiesta. Dopo averla salvata in memoria, dobbiamo estrarre le informazioni che ci servono dalla command line:
                GET /index.html HTTP/1.1
            Di conseguenza le informazioni necessarie sono:
                - il metodo di richiesta, come GET o POST (char * method)
                - la risorsa - il file - richiesto, ovvero l'URI (char * file_name)
                - la versiona HTTP della richiesta (char * version)
            L'idea risolutiva è molto semplice in quanto sappiamo che la command line avrà sempre la stessa struttura. La prima cosa che dobbiamo estrarre è il metodo. Inizializziamo quindi il puntatore *method uguale al puntatore *command_line: in uuesto modo *method punta al primo carattere della richiesta. In secondo luogo iniziamo un loop che scorre *command_line e si ferma quando si trova uno spazio. In questo modo sappiamo che abbiamo terminato il valore di *method. Terminato il ciclo for, null-terminiamo *method e incrementiamo la variabile i, che verrà utilizzata per estrarre *file_name e *version.
        */

        // inizializzo metodo che punta al'inizio della command_line
        method = command_line;      // = command_line[0]
        
        // inizializzo i
        i = 0;

        // mi fermo quando trovo uno spazio 
        while( command_line[i] != ' ' )
            i++;

        // null-termino method
        command_line[i++] = 0;

        /* estraggo file_name nello stesso modo */
        file_name = command_line + i;   

        while( command_line[i] != ' ' )
            i++;

        command_line[i++] = 0;


        /* estraggo anche version */
        version = command_line + i;     
        
        // cambio da ' ' a 0 per il fine riga (abbiamo messo il terminatore nel parsing precedente)
        while( command_line[i] != 0 )
            i++;

        command_line[i++] = 0;

        printf("Method = %s\nURI = %s\nVersion = %s\n\n\n", method, file_name, version);



        /*
            Ora che abbiamo tokenizzato la richiesta, sappiamo quale file prendere e restituirne il contenuto tramite la response.
            Per aprire il file si usa la funzione fopen(). In particolare vogliamo far si che non sia possible accedere ai file che stanno sopra nel percorso assoluto. A questo proposito inseriamo uno spazio: filename + 1. In secondo luogo inseriamo anche le modalità di accesso al file: "rt", ovvero Read Text file.
            Se il file non esiste la funzione ritorna NULL, è quindi necessario notificare il Client del fatto che la risorsa è inesistente: si crea quindi una response indicando le problematiche trovate (404 NOT FOUND) e la si manda al cliente tramite una write().  In seguito si chiude sia il socket tramite close() e si uccide il processo. Non è necessario fare un fclose() perchè in questo caso il file nemmeno si apre.
        */
        
        file = fopen(file_name + 1, "rw");

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


        /*
            Se il file esiste è ovviamente necessario mandare il file richiesto. Inviamo prima l'header della response indicando che il file esiste.
            In secondo luogo iteriamo sul file finchè non arriviamo alla fine tramite un while loop. Per ciascuna itrazione, tramite la funzione fread(), leggiamo 1000 bytes dal file e li inseriamo all'interno del buffer entity[] il quale verrà immediatamente utilizzato per essere mandato al
        */

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