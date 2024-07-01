#include <stdlib.h>
#include <stdio.h>
#include <errno.h>                  // errno
#include <unistd.h>                 // read, write, fork
#include <string.h>                 // strlen





int main(char * argc, char * argv[], char * env[]){

    int i, j, t;            // definisco indici e una variabile temporanea
    int length = 0;         // lunghezza dell'entity-body
    char * key;
    char * value;
    char * buffer;

    /*
        Scorro environment e lo parso in modo tale che separi il nome della chiave dal suo valore.
        Durante il parsing salva il valore della CONTENT_LENGTH
    */
    for(i = 0; env[i]; i++){
        
        // ricavo il chiave
        key = env[i];

        // trovo il separatore tra chiave e valore
        j = 0;
        while( env[i][j] != '=' )
            j++;
        
        // null termino
        env[i][j] = 0;
        
        // ricavo il valore
        value = env[i] + j + 1;

        // se la trovo, mi salvo la CONTENT_LENGTH. Se non è zero, i forloop sotto partono
        if( strcmp(key, "CONTENT_LENGTH") )
            length = atoi(value);

    }

    fflush(stdout);

    // riservo memoria
    buffer = malloc(length);

    // lettura dell'entity-body della request dal socket 0, ovvero lo standard input
    for(i = 0; i < length && (t = read(0, buffer + i, length - i)); i += t)

    printf("Body received\n\n");

    // scrittura dell'entity-body della response nel socket 1, ovvero lo standard output
    for(i = 0; i < length && (t = write(1, buffer + i, length - i)); i += t)

    // stampa variabili di environment
    for(i = 0; env[i]; i++)
        printf("%s —————> %s\n", env[i], env[i] + strlen(env[i]) + 1);


    return 0;

} // main
