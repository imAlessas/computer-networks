#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // read, write, fork
#include <string.h>                 // strlen





struct sockaddr_in server_addr;     // struct per definire l'indrizzo del server
struct sockaddr_in client_addr;     // struct per definire l'indrizzo del client remoto



// definisco il buffer che conterrà i campi dell'header così come arrivano, da cui leggiamo i caratteri
char hbuf[10000];

// è un array di 'coppie' (una mappa) che puntano ciascuna ad il nome di un campo e al suo valore nell'header presente in hbuf 
struct headers{
    char * n;   // nome
    char * v;   // valore
} h[100];       // definisco la tabella di indicizzazione



int main(){

    // definizione di variabili locali
    int s, s_double;                    // socket
    int t;                              // variabile temporanea
    int len;                            // lunghezza 
    char request_buffer[3001];          // buffer per consumare request
    int i, j;
    char * commandline;



    // crea il socket
    s = socket( AF_INET, SOCK_STREAM, 0 );
    // printf("Socket: %d\n", s);

    // terminazione nel caso di errori
    if(s == -1){
        printf("ERRNO = %d (%d)\n", errno, EAFNOSUPPORT);
        perror("Socket fallita\n");
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
    server_addr.sin_port        = htons(31415);
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
        printf("ERRNO = %d (%d)\n", errno, EAFNOSUPPORT);
        perror("Bind fallita\n");
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
        printf("ERRNO = %d (%d)\n", errno, EAFNOSUPPORT);
        perror("Listen fallita\n");
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

    len = sizeof(struct sockaddr);

    while(1){
        
        // accetto la prima richiesta di connessione della coda
        s_double = accept(s, (struct sockaddr *) &client_addr, &len);

        // se siamo nel processo padre (fork != 0) ritorniamo su accept()
        if(fork()) continue;


        /* da qui in poi ci sono solo processi figli */


        // terminazione nel caso di errori
        if(s_double == -1){
            printf("ERRNO = %d (%d)\n", errno, EAFNOSUPPORT);
            perror("Accept fallita\n");
            return 1;
        }



        // consumo la richiesta in maniera conforme alla grammatica
        // codice molto simile al consume della response web-client/HTTP-1.1.c

        commandline = h[0].n = hbuf;
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






        // creo la response
        char response[2000000];
        printf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n<html><h1>Hello World!</h1></html>");
        
        // invio la response al client mediante la write()
        write(s_double, response, strlen(response));

        // chiudo il socket
        close(s_double);

        // evito creazione di processi zombie
        exit(1);
    }






    return 0;

} // main