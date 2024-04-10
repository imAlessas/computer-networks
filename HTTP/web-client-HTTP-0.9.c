
#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // write
#include <string.h>                 // strlen






int main(){

    // definizione di variabili locali
    struct sockaddr_in server_addr;     // struct per definire l'indrizzo del server
    int s;          // socket
    int t;          // variabile temporanea
    unsigned char * p;          // puntatore per indirizzo IP

    /*
        Creiamo un socket:
            → AF_INET       : servizio della famiglia IP4
            → SOCK_STREAM   : rappresenta lo stream
            → 0             : protocollo 0

        Il valore di ritorno della funzione socket() è un file descriptor. Stampando a video l'intero, notiamo che stampa a schermo 3. Se però si aprono più socket, si osserva che il valore dei socket crescono, partendo da 3. Questo ci suggerisce che è un indice, in particolare rappresenta la tebella di sistema contentente tutti gli stream. I tre indici prima di 3 sono rispettivamente:
            • Std input
            • Std output
            • Std error, flusso anch'esso, di default è a schermo ma è diviso nel caso in cui si vogliano direzionare i flussi di errori da un'altra parte

            Se si apre uno stream su file prima di aprire la socket:
                FILE * f = fopen("file.txt", "w");
            Si osserva che il valore di s, da 3 passa a 4 parchè abbiamo aperto un altro flusso prima di s
    */
    s = socket( AF_INET, SOCK_STREAM, 0 );
    // printf("Socket: %d\n", s);

    /*
        Se la funzione non ha successo ritorna '-1' e si ha un 'errno', che è una variabile globale che identifica il numero di errore. La chiamata di libreria, detta 'perror' che prende una stringa e ci aggiunge il messaggio di errore a sistema.
        Per far scatenare un errore è sufficient inserire un numero casuale al posto di uno dei campi della funzione socket in quanto tali varibili rappresentano dei numeri interi.
    */
    if( s == -1){
        printf("ERRNO = %d", errno);
        perror("Socket fallita");
        return 1;
    }

    /*
        Ora che abbiamo il socket, dobbiamo aprire lo stream. Per farlo, dobbiamo prendere l'iniziativa (perchè così è strutturato il modello Client-Server) attraverso una request al server. Tale richiesta viene mandata in formato stream. È necessario aprire uno stream verso il server. Come facciamo a raggiungere il server? 
        Avendo scelto il socket adibito per una connessione TCP, ci vengono messe a disposizione delle chiamata a sistema che verranno utilizzate per creare applicazione client, server e proxy. Quella di nostro interesse è la funzione connect().
        La funzione connect() apre uno stream, una connessione TCP nei confronti di un server remoto. In particolare, questa chiamata a sistema connette il socket ad un indirizzo. Questi sono specificati come segue
            → int sockfd, ovvero il file descriptor del socket
            → const struct sockaddr *addr
            → socklen_t addrlen

        Cerchiamo ora di capire come gestire la struct sockaddr, descritta anche in 'man 7 ip' come segue:
            struct sockaddr_in{
                sa_family_t         sin_family;     // famiglia
                in_port_t           sin_port;       // 
                struct in_addr      sin_addr;       // indirizzo internet
            }
        Prima di tutto va osservato che il tipo 'struct sockaddr_in' è diverso da 'struct sockaddr'. Questo perchè l'interfaccia Linux deve valere per tutto, tanto che questa funzione non fa alcun riferimento al TCP. Abbiamo quindi degli input spcifici su una funzione generica: problema tipico di polimorfismo. Questo problema è stato risolto mediante i puntatori in quanto questi, qualnque sia il contenuto dello cella, contiene sempre e solo l'indirizzo. Però il puntatore è di tipo 'struct sockaddr' che è un tipo generale che non si usa mai. Questo tipo contiene solo un campo, ovvero un numero intero che specifica il tipo di struttura sockaddr specializzata, ovvero la famiglia. Nel momento in cui sin_family = AF_INET, si determina il modo in cui gli altri due campi della struct verranno letti.
        In secondo luogo la funzione necessita di un indirizzo. L'indirizzo IP è composto da 4 campi che in decimale vanno da 0 a 255 e si traducon in 8 bit. Questi 4 valori in memoria sono salvati nel modo in cui si leggono: 147.162.2.100 all'indirizzo x sarà contenuto 147, all'indrizzo x + 1 sarà contenuto 162 e così via. Si identifca quindi un'interfaccia di rete connessa ad internet. L'instradamento dell'informazione, quindi tutte le procedure necessarie per far passare un dato da un server ad un'altro con determinati indirizzi IP è risolto dal livello 3.
        Infine è necessaria la porta. Questo perchè nel modello Client-Server, il server non è altro che un programma eseguito. L'indirizzo di rete non basta in quanto ci porta alla macchina server, a noi serve l'effettivo programma. È come avere l'indirizzo di un condominio ma non conoscere l'effettiva porta dell'appartamento giusto. Non a caso il terzo campo è la porta: questa ci porta ad un end-point - ad un socket - che è un file descriptor a sua volta.
        → Per indirizzare un programma (server) che gira sulla macchina server, oltre all'indirizzo IP è anche necessario il port, ovvero un numero a 16bit, unico all'interno della macchina server, specifico per un tipo di servizio. Inoltre, i programmi server utilizzano dei numero di port, dette "well-known", che identificano il tipo di protocollo utilizzato. Nel nostro caso, per il servizio HTTP è necessaria la porta 80. [/etc/services]

    */

    // imposto i campi della struct sockaddr_in del server    
    server_addr.sin_family = AF_INET;       // la famiglia qualificherà il tipo specifico
    
    /*
        Alcune architetture sono BigEndian, altre LittleEndin; si è scelto che il network order è BigEndian. In una macchina Linux, è necessario utilizzare la funzione htons() function: Hosto-TO-Network-Short.
    */
    server_addr.sin_port = 80;              // la porta a cui ci vogliamo collegare
    
    /*
        Per capire quale indirizzo inserire per fare una richiesta a Google, è necessario digitare sul terminale:
            nslookup www.google.com → mostra l'indirizzo del server di Google, in questo caso è 142.250.187.196
        Come rappresentare un indirizzo IP? La soluzione è utilizzare un puntatore di tipo char (quindi che occupa 8 bit) e poi accedere alle 4 celle, ciascuna delle quali definisce una parte dell'indirizzo IPv4. Si osserva che in questo caso non è necessario fare la conversione Host-TO-Network perchè già sono stati salvati nell'ordine giusto
        Se inserisco un indirizzo IP erratto, il programma si mette in attesa della CONFIRM in quanto è una chiamata bloccante ad funzione asincrona. Dopo un po' di tempo, se l'indirizzo è inesistente, il programma termina dopo che il time-out scade. In altri casi la richiesta può essere direttamente rifiutata, si può provare inserendo come indirizzo 127.0.0.1 con una port non aperta. Ciò nonostante questa casistica è sempre più rara perchè sempre più spesso sono implementati dei firewall che bloccano una richiesta indesiderata ancora prima che raggiunga il server
    */
    p = (unsigned char *) &server_addr.sin_addr.s_addr;
    p[0] = 142;     p[1] = 250;     p[2] = 187;      p[3] = 196;

    // per invocare la connect è necessario passare un puntatore a server_addr castato come sockaddr in modo da gestire il problema del polimorfismo con i puntatori
    t = connect(s, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));

    
    // Per capire se la connect ha successo si controlla che il suo valore di ritorno sia diverso da -1
    if(t == -1){ 
        perror("Connessione fallita");
        return 1;
    }


    /*
        Per utilizzare un socket, quindi per poter mandare a Google una stream si usano le stesse chiamate a sistema che si usano per scrivere sui file.
    
        La funzione write() richiede un file un file_descriptor - che vale sia se è un socket, oppure un file -, un buffer e la dimensione.
        Si osserva che la requesta ha due caratteri per indiricare il fine riga:
            • \r che indica il carriage return
            • \n che indica la nuova linea
    */
    char * request = "GET /\r\n";
    write(s, request, strlen(request));

    /*
        Dopo aver mandato la stringa, si deve leggera la richiesta. La funzione read(), come la connect(), è bloccante. Ciò significa che finchè il server non mi risponde rimango in attesa e blocco l'esecuzione
        
    */









    return 0;

} // main
