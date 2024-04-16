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
} h[100];       // definisco la tabella di indicizzazione dei campi dell'header





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
        Facciamo una richiesta al server attraverso l'HTTP 1.0. La grammatica della simple-request è definita nel modo seguente:
            Method SP Request-URI CRLF
        → Method:       che può essere "GET", "HEAD" oppure "POST"
        → SP:           space
        → Request-URI:  acronimo di Unifor Request Identifier, identifica la risorsa verso cui applicare la richiesta
        → CRLF:         dato da \r [carriage return] e \n [new line]

        Scegliendo come richiesta "GET / HTTP/1.0\r\n", si ossera che la richiesta non funziona, in particolare è necessario aggiungere un secondo CRLF. Il primo CRLF è alla fine della simple-requesta ma, nella grammatica, è indicato che è necessario un secondo CRLF al fine di dividere l'HEADER della requesta dal suo BODY.
        Si osserva infine che se mettiamo HTTP/1.1, la richiesta non funziona perche l'HTTP 1.1 necessita di un altro tipo di grammatica
    */
    char * request = "GET / HTTP/1.0\r\n\r\n";
    write(s, request, strlen(request));

    /*
        Controlliamo ora che la response sia conforme alla grammatica definita nell'RFC 1945:
            Simple-Response | Full-Response
        La Simple-Response non è altro che quella di Berners Lee e contiene solamente il Entity-Body:
            Simple-Response = [ Entity-Body ]

        La Full-Response invece è composta da un header, opzionale, che può essere di tre tipi e dall'effettivo Entity-Body:
            Full-Response   = Status-Line
                            *( General-Header
                            | Response-Header
                            | Entity-Header )
                            CRLF
                            [ Entity-Body ]
        Prima di tutto osserviamo che la struttura è molto simile alla request. In secondo luogo, ancora più interessante è che l'Entity-Body è opzionale. Questo ha senso perchè in alcune richiest si vuole conoscere solo lo stato, non necessariamente anche l'Entity-Body.

        Andiamo ora a capire meglio come la full request è strutturata.
        Nella status line, la prima infromazione è la versione in quanto non deve più sottostare al vincolo di Berners Lee. In secondo luogo, imponendo la versione come prima informazione, la response non potrà mai essere quella di Berners Lee (non viene quindi manteuta la retrocompatibilità). Dopo la versione è presente lo Status-Code con una frase che esplicita l'errore. 
            Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
                Status-Code
                    • 1xx: Informational -  Not used, but reserved for future use
                    • 2xx: Success -        The action was successfully received, understood, and accepted.
                    • 3xx: Redirection -    Further action must be taken in order to complete the request
                    • 4xx: Client Error -   The request contains bad syntax or cannot be fulfilled (i.e. 404)
                    • 5xx: Server Error -   The server failed to fulfill an apparently valid request
            La Reason-Phrase è stata aggiunte al fine di dare un semplice e rapido debugging, vale anche per il protocollo della posta (SMTP). Questo complica comunque le cose perchè sarà necessario un pareser al fine di 

        Osserviamo che dopo aver stampato a schermo la response, la prima riga è proprio la Status-Line che contiene esattamente la HTTP-Version, lo Status-Code e la sua Reason-Phrase:
            HTTP/1.0 200 OK
    */

    /*
        Ora che conosciamo meglio la teoria, il prossimo passo consiste nel fare il parsing dell'header della risposta, tenendo conto che è uno stream e che dobbiamo farlo nel modo più efficiente possibile. È neecessario creare un parser che prima consumi tutto l'header e poi tutto l'Entity-Body: la difficoltà sta nel fatto che non esiste un carattere che riesca perfettamente a delineare la fine dell'Header con l'inizio dell'Entity-Body (c'è il CRLF, ma non è l'unica occorrenza). In secondo luogo il parser deve effettuare le operazione durante la ricezione: la scelta di attendere prima tutto lo stream e in secondo luogo analizzarlo crea una latenza non indifferente.
        L'obiettivo del parser è quello di ottenere una tabella che classifica il nome-valore dell'Header a partire dai caratteri HTTP che mi arrivano e che raccolgo in un buffer. Si sttolinea inoltre che non si vuole copiare i dati in arrivo dal buffer alla tabella in quanto presenterebbe una inefficienza; piuttosto si sceglie di strutturare il buffer stesso.
        Per implementare una tabella Nome-Valore si potrebbe pensare ad una mappa, ma questa è una struttura ad alto livello, poco compatibile con il C. Definiamo quindi la seguente struttura con due puntatori char
            struct headers{
                char * n;   // nome
                char * v;   // valore
            }
        In particolare, al fine di creare la tabella on-the-fly, devo riconoscere che, per esempio, nello stream
            Date: Sun, 14 Apr 2024 13:33:36 GMT
            h[0].n = "Date"
            h[0].v = "Sun, 14 Apr 2024 13:33:36 GMT"
        Ma affinche siano delle stringhe, è necessario inserire un TERMINATORE (\0) alla fine di ciascuna delle due stringhe al momento giusto,
    */

    // inizializzo il primo puntatore di h[0].n al primo carattere del buffer dell'header (hbuf). Essendo la prima riga, questa è la status line
    statusline = h[0].n = &hbuf;

    j = 0;

    // leggo un carattere alla volta
    for(i = 0; read(s, hbuf + i, 1); i++ ){
        /*
            Nel momento in cui troviamo un separatore, che può essere ':' oppure CRLF, concludiamo un token e puntiamo a quello successivo
                → Nel primo caso, quando troviamo il fine riga, mettiamo il terminatore al posto di '\r'
                → Nel secondo caso [':'] mettiamo il terminatore al posto di ':' e indicizziamo 
            Inoltre per gestire la fine di un header, che è delimitato da due fine riga (CRLFCRLF) basta pensare che l'ultimo campo dell'header è nullo, che è proprio quello che succede duerante il ciclo.

        */

        // fine campo header
        if( hbuf[i - 1] == '\r' && hbuf[i] == '\n'){
            
            hbuf[i - 1] = 0;            // terminatore su \r
           
            /*
                La linea di codice sopra nullifica la stringa precedente e quindi fa si che, alla fine dell'header esista un campo nullo, quindi h[j].n[0] == 0.
                Se ciò occorre significa che siamo arrivati alla fine dell'header e possiamo quindi interromper il ciclo con un break.
                È necessario mettere questo prima della prossima istruzione perchè incrementare j non ha senso dato che se è finito l'headere non sono più presenti righe nella tabella.
            */
            if( !( h[j].n[0] ) )    // entro se sono alla fine dell'header
                break;
            
            /*
                Mi trovo alla fine di un campo header, di conseguenza il carattere successivo, ovvero hbuf[i + 1] indica l'inizio del nome del prossimo campo dell'header.
                A questo proposito incremento j e accedo alla nuova riga della tabella che conterrà l'indirizzo al nome del campo 
            */
            h[++j].n = &hbuf[i + 1];    // imposto il nome della nuova riga della tabella
        }

        /*
            Fine nome campo header. 
            Dobbiamo stare però attenti alla datail quanto il valore contiene dei ':', rendendo non sufficiente il controllo hbuf[i] == ':' [Date —————> 37 GMT]
            È quindi necessario inserire un controllo aggiuntivo per verificare che h[j].v == NULL. Ciò significa il puntatore al valore viene inserito solo quando il valore è NULL, ovvero prima del primo inserimento, altrimenti il puntatore non viene aggiornato, mantenendo integra l'intera stringa [Tue, 16 Apr 2024 14:59:24 GMT]
        */
        if( (hbuf[i] == ':') && (h[j].v == NULL) ){
            /*
                Mi trovo alla fine del nome di un campo nell'header di conseguenza il carattere successivo indica l'inzio del valore del campo con nome h[j].n
            */
            h[j].v = &hbuf[i + 1];      // imposto il valore della tabella
            hbuf[i] = 0;                // terminatore
        }
    }

    // stampo la tabella di indicizzazione, j contiene il numero di righe della tabella
    for(i = 0; i < j; i++)
        printf("%s —————> %s\n", h[i].n, h[i].v);
    printf("\n\n");



    // mi preparo per leggere l'Entity-Body
    const int RESPONSE_SIZE = 2000000;      // dimensione del buffer
    char response[RESPONSE_SIZE];           // buffer per la consumare il Body
    
    // leggo l'Entity-Body
    for ( i = 0; t = read(s, response + i, RESPONSE_SIZE - 1 - i); i += t ) {}

    response[i] = 0;    // inserisco il terminatore alla fine del body

    print("%s\n\n", response);



    return 0;

} // main
