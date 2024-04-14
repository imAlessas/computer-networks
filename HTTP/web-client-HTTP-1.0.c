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

    // leggo la risposta
    const int RESPONSE_SIZE = 2000000;      // dimensione del buffer
    char response[RESPONSE_SIZE];           // buffer per la risposta
    
    for ( i = 0; t = read(s, response + i, RESPONSE_SIZE - 1 - i); i += t ) {}

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

    response[i] = 0;
    print("%s\n\n", response);

    /*
        Il prossimo passo consiste nel fare il parsing della risposta, tenendo conto che è uno stream e che dobbiamo farlo nel modo più efficiente possibile. È neecessario creare un parser che prima consumi tutto l'header e poi tutto l'Entity-Body.
    */


    return 0;

} // main
