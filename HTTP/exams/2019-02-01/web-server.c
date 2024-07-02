#include <stdio.h>  // printf, perror, fopen, fread, feof, fclose
#include <string.h> // strlen
#include <stdlib.h> // exit
#include <unistd.h> // read, write, fork
#include <sys/socket.h> // socket, bind, listen, accept
#include <arpa/inet.h>  // htons, sockaddr, sockaddr_in

// constants
#define PORT 33209
#define BUFFER_SIZE 1024
#define BLACKLIST "blacklist.txt"


struct char_map {
    char * key;
    char * value;
};



int main() {

    // local variables
    int s, s_double;                    // sockets
    char * command_line;                // first line of request
    struct char_map headers[100];       // headers
    char header_buffer[BUFFER_SIZE];    // header buffer, here there will be all the info from the header
    char response_buffer[BUFFER_SIZE];  // response buffer, will be used to temporarily store the response
    char blacklist_buffer[BUFFER_SIZE]; // blacklist buffer, will be used to temporarily store the blacklist
    char * method, * uri, * version;    // parsed values from command_line
    int i;                              // generic index
    char * host, * referer; // parsed values
    char link[200];


    // define address
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;





    // socket
    s = socket( AF_INET, SOCK_STREAM, 0);

    // terminate if error
    if( s == -1 ) {
        perror("socket() failed");
        return 1;
    }

    
    // define address
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(PORT);
    server_address.sin_addr.s_addr = 0;

    // bind
    if( -1 == bind(s, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in)) ) {
        perror("bind() failed");
        return 1;
    }

    // listen
    if( -1 == listen(s, 5) ) {
        perror("listen() failed");
        return 1;
    }


    int sockaddr_size = sizeof(struct sockaddr);

    while(1) {
        
        // accept
        s_double = accept(s, (struct sockaddr *) &client_address, &sockaddr_size);


        // create sub-process
        if (fork()) {
            close(s_double);
            continue;
        }

        // terminate if error
        if( s_double == -1 ) {
            perror("accept() failed");
            return 1;
        }


        // parse the header
        command_line = headers[0].key = header_buffer;
        int lines = 0;

        for(i = 0; read(s_double, header_buffer + i, 1); i++) {

            // end of the line
            if(header_buffer[i - 1] == '\r' && header_buffer[i] == '\n') {
                
                // null-terminate
                header_buffer[i - 1] = 0;

                // check if it is the end
                if( !headers[lines].key[0] )
                    break;

                // create new line on the headers
                lines++;
                headers[lines].key = &header_buffer[i + 1];
            }

            if( header_buffer[i] == ':' && (headers[lines].value == NULL)) {
                
                // start value
                headers[lines].value = &header_buffer[i + 1] + 1;

                // null-terminate
                header_buffer[i] = 0;

            }
        }

        // print headers
        for(i = 0; i < lines; i++)
            printf("%s —————> %s\n", headers[i].key, headers[i].value);
        
        // retrieve host
        for (i = 0; i < lines; i++)
            if (!strcmp(headers[i].key, "Host"))
                host = headers[i].value;
        
        // retrieve referer
        for (i = 0; i < lines; i++)
            if (!strcmp(headers[i].key, "Referer"))
                referer = headers[i].value;


        // parse method, uri, version
        method = command_line;
        for(i = 0; command_line[i] != ' '; i++);
        command_line[i++] = 0;

        uri = command_line + i;
        for(; command_line[i] != ' '; i++);
        command_line[i++] = 0;

        version = command_line + i;
        for(; command_line[i] != 0; i++);
        command_line[i++] = 0;

        // print values
        printf("Method —————> %s\nURI —————> %s\nVersion —————> %s\n", method, uri, version);


        // opens file
        FILE * file = fopen(uri + 1, "rw");

        if( file == NULL ) {
            
            // create 404 response
            sprintf(response_buffer, "HTTP/1.1 404 NOT FOUND\r\n\r\n<html><h1>File %s was not found.</h1></html>", uri);
            
            // send response
            if( -1 == write(s_double, response_buffer, strlen(response_buffer)) ){
                perror("write() failed");
                return 1;
            }

        } else {
            
            // create link
            sprintf(link, "%s%s", host, uri);

            // open blacklist
            FILE * blacklist = fopen(BLACKLIST, "r");
            char * blacklist_item;

            // retrive link from blacklist.txt if exists
            while (fgets(blacklist_buffer, BUFFER_SIZE, blacklist)) {

                // null terminate
                blacklist_buffer[strlen(blacklist_buffer) - 1] = 0;

                // if uri is in the blacklist
                if ( !strncmp(blacklist_buffer, link, strlen(link)) ) {



                    snprintf(response_buffer, BUFFER_SIZE, "HTTP/1.1 403 Forbidden\r\nConnection:close\r\n\r\n"
                                                            "<html>"
                                                            "<head><meta http-equiv=\"refresh\" content=\"2;URL=%s\"></head>"
                                                            "<h1>You are not allowed to access in this page because it is blacklisted</h1></html>",
                                                            referer ? referer : "index.html");

                    if( -1 == write(s_double, response_buffer, strlen(response_buffer)) ){
                        perror("write() failed");
                        return 1;
                    }

                    for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;

                    // close socket and kill process
                    close(s_double);
                    exit(1);

                }

                for(i = 0; i < BUFFER_SIZE; i++) blacklist_buffer[i] = 0;

            }

            // send accept header
            sprintf(response_buffer, "HTTP/1.1 200 OK\r\n\r\n");

            if( -1 == write(s_double, response_buffer, strlen(response_buffer)) ){
                perror("write() failed");
                return 1;
            }

            // read and send the file
            while( !feof(file) ) {
                
                // read 1Kb from the file
                fread(response_buffer, 1, 1024, file);

                // write the answer
                if( -1 == write(s_double, response_buffer, strlen(response_buffer)) ){
                    perror("write() failed");
                    return 1;
                }

                for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;

            }

            fclose(file);

        }

        
        printf("\n\n\n");

        // close socket and kill process
        close(s_double);
        exit(1);

    }

    return 0;

} // main