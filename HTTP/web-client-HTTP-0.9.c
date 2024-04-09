
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>






int main(){
    // definizione di variabili locali
    struct sockaddr_in server_addr;     // struct per definire il server
    int s;          // socket
    int t;          // tile temporaneo
    


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
        return -1;
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

    */

    // imposto i campi della struct sockaddr_in del server    

    t = connect(s, )




    return 0;

} // main
