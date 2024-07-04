#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // write
#include <string.h>                 // strlen, strcmp
#include <stdlib.h>                 // atoi





char hbuf[10000];

struct headers{
    char * n; 
    char * v; 
} h[100];     



int main(){

    // local varibles
    struct sockaddr_in server_addr;     // server address
    int s;                              // socket
    int t;                              // temporary
    unsigned char * p;                  // ip address piointer
    int i, j;
    char * statusline;


    // create socket
    s = socket( AF_INET, SOCK_STREAM, 0 );
    // printf("Socket: %d\n", s);

    if( s == -1){
        printf("ERRNO = %d (%d)\n", errno, EAFNOSUPPORT);
        perror("Socket fallita\n");
        return 1;
    }


    /* Setup for request */

    // set server addr    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    
    // IPv4 server
    p = (unsigned char *) &server_addr.sin_addr.s_addr;
    p[0] = 142;     p[1] = 250;     p[2] = 187;      p[3] = 196;


    // connect server
    if(-1 == connect(s, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in))){ 
        perror("Connessione fallita\n");
        return 1;
    }


    // send request
    char * request = "GET / HTTP/1.1\r\n\r\n";
    write(s, request, strlen(request));



    statusline = h[0].n = hbuf;
    j = 0;

    // reade header
    for( i = 0; read(s, hbuf + i, 1); i++ ){

        // end of line
        if( hbuf[i - 1] == '\r' && hbuf[i] == '\n'){
            
            hbuf[i - 1] = 0;
           
            if( !( h[j].n[0] ) )
                break;
            
            h[++j].n = &hbuf[i + 1];
        }

        // end of name
        if( (hbuf[i] == ':') && (h[j].v == NULL) ){

            h[j].v = &hbuf[i + 1];
            hbuf[i] = 0;
        }
    }

    // print headers
    for(i = 0; i < j; i++)
        printf("%s —————> %s\n", h[i].n, h[i].v);
    printf("\n\n");



    //  get content length
    int content_length;

    for(i = 0; i < j; i++)
        if( !strcmp( h[i].n , "Content-Length" ))
            content_length = atoi(h[i].v);
    

    // get entity body
    char response[2000000];           
    
    if ( !content_length ){

        // read the response
        for ( i = 0; t = read(s, response + i, content_length - i); i += t ) {}

        // null-terminate
        response[i] = 0;
        printf("%s\n\n", response);

        // exit
        return 0;
   
    }
        

    long chunk_size;
    char chunk_buffer[8];

    // will contain all the read bytes
    j = 0;

    do {    // when chunk_size == 0, exit

        // consume and convert in dec the chunk size
        for(chunk_size = 0, i = 0;
            read(s, chunk_buffer + i, 1)  &&  !(chunk_buffer[i - 1] == '\r' && chunk_buffer[i] == '\n');
            i++) {
            
            // lower case conversion
            if( chunk_buffer[i] >= 'A' && chunk_buffer[i] <= 'F')
                chunk_buffer[i] = chunk_buffer[i] - ('a' - 'A');

            
            // letter to number conversion
            if( chunk_buffer[i] >= 'a' && chunk_buffer[i] <= 'f')
                chunk_size = chunk_size * 16 + chunk_buffer[i] - 'a' + 10;
            
            // number conversion
            if( chunk_buffer[i] >= '0' && chunk_buffer[i] <= '9')
                chunk_size = chunk_size * 16 + chunk_buffer[i] - '0';
        
        }

        // read chunk 
        for( i = 0; t = read(s, response + j, chunk_size - i); i += t, j += t);

        // read CRLF
        read(s, chunk_buffer, 2);

    } while( chunk_size );


    // null-terminate response
    response[j] = 0;

    printf("%s\n\n", response);


    return 0;

} // main
