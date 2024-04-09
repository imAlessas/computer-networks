
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>






int main(){
        // definizione di variabili locali



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
        int s = socket( AF_INET, SOCK_STREAM, 0 );
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
        */



        return 0;

} // main
