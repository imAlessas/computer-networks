#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>



struct char_map{
    char * key;
    char * value;
};





int main() {
    
    const int PORT = 9278;
    const char CRLF[] = "\r\n";
    const int BUFFER_SIZE = 10 * 1024;

    
    // local variable definition
    int s, s_double;                        // socket
    int t;                                  // temporary variable
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    char buffer[BUFFER_SIZE];                 // generic buffer
    struct char_map headers[100];           // will contain the parsed headers
    char * command_line;                    // first line of the request
    int i;                                  // generic index




    // create socket
    s = socket( AF_INET, SOCK_STREAM, 0 );
    
    // terminate if error
    if(s == -1) {
        perror("socket() failed");
        return 1;
    }


    // set up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = 0;


    // bind socket with server address
    t = bind(s, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in));

    // terminate if error
    if(t == -1){
        perror("bind() failed");
        return 1;
    }


    // put socket s in listen mode
    t = listen(s, 5);

    // terminate if error
    if(t == -1){
        perror("listen() failed");
        return 1;
    }


    // start server
    int address_length = sizeof(struct sockaddr);

    while(1) {
        
        // dequeue the backlog and process the request into a new socket
        s_double = accept(s, (struct sockaddr *) &client_address, &address_length);

        if(fork()) continue;

        // terminate if error
        if(s_double == -1) {
            perror("accept() failed");
            return 1;
        }

        command_line = headers[0].key = buffer;
        int lines = 0;

        for( i = 0; read(s_double, buffer + i, 1); i++) {

            // end of the line
            if( buffer[i - 1] == '\r' && buffer[i] == '\n'){
                
                // null-terminate
                buffer[i - 1] = 0;

                if( !headers[lines].key[0] )
                    break;

                // set-up new line
                lines++;
                headers[lines].key = &buffer[i + 1];

            }

            // endo fo the name, start of the value
            if( buffer[i] == ':' && headers[lines].value == NULL) {
                
                // set header value
                headers[lines].value = &buffer[i + 1];

                // null-terminate
                buffer[i] = 0;

            }

        }

        // extract important info from command_line
        char * method, * uri, * version;
        
        method = command_line;
        for(i = 0; command_line[i] != ' '; i++);
        command_line[i++] = 0;

        uri = command_line + i;
        for(; command_line[i] != ' '; i++);
        command_line[i++] = 0;

        version = command_line + i;
        for(; command_line[i] != 0; i++);
        command_line[i++] = 0;

        for(int i = 0; i < lines; i++)
            printf("%s ----> %s\n", headers[i].key, headers[i].value);
        printf("Method ----> %s\nURI ----> %s\nVersion ----> %s\n\n\n", method, uri, version);



        // display the requested file
        FILE * file = fopen(uri + 1, "rw");

        for(int i = 0; i < BUFFER_SIZE; i++)
            buffer[i] = 0;

        if(file == NULL) {
            
            // create response
            sprintf(buffer, "HTTP/1.1 404 NOT FOUND\r\nConnection: close\r\n\r\n"
                            "<html><body>"
                            "<h1>404 Not Found</h1>"
                            "<p>Sorry, the file <strong>%s</strong> was not found on this server.</p>"
                            "</body></html>", uri);
            
            printf("%s\n", buffer);
            
            // write response
            if( write(s_double, buffer, strlen(buffer)) == -1 ) {
                perror("write() failed");
                return 1;
            }

        } else {
            
            // create response header
            sprintf(buffer, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            
            // write response
            if( write(s_double, buffer, strlen(buffer)) == -1 ) {
                perror("write() failed");
                return 1;
            }

            char chunk_size[20];

            while( !feof(file) ){
                
                // read at most 1Kb from the file
                fread(buffer, 1, 1024, file);

                // get first line of the response
                sprintf(chunk_size, "%x\r\n", strlen(buffer));

                // write the first line
                if( write(s_double, chunk_size, strlen(chunk_size)) == -1 ) {
                    perror("write() failed");
                    return 1;
                }

                // write the chunk
                if( write(s_double, buffer, strlen(buffer)) == -1 ) {
                    perror("write() failed");
                    return 1;
                }

                // end of the chunk
                if( write(s_double, CRLF, strlen(CRLF)) == -1 ) {
                    perror("write() failed");
                    return 1;
                }

            }

            // last chunk
            sprintf(buffer, "0\r\n");

            // write last chunk
            if( write(s_double, buffer, strlen(buffer)) == -1 ) {
                    perror("write() failed");
                    return 1;
            }


        }

        // close socket and kill thread
        close(s_double);
        exit(1);

    }


    return 0; 

} // main
