#include <stdio.h>  // printf, perror, fopen, fread, feof, fclose
#include <string.h> // strlen
#include <stdlib.h> // exit
#include <unistd.h> // read, write, fork
#include <sys/socket.h> // socket, bind, listen, accept
#include <arpa/inet.h>  // htons, sockaddr, sockaddr_in
#include <netdb.h> // gethostbyname


// constants
#define PORT 58141
#define BUFFER_SIZE 1024



struct char_map {
    char * key;
    char * value;
};



int main() {

    // local variables
    int i, t;                                        // generic index, generic variable
    int s, s_double, s_remote;                       // sockets
    char * command_line;                             // first line of request
    struct char_map headers[100] = {{NULL, NULL}};   // headers
    char header_buffer[BUFFER_SIZE] = {0};           // header buffer, here there will be all the info from the header
    char request_buffer[BUFFER_SIZE] = {0};          // request buffer, will be used to store and send the request
    char response_buffer[BUFFER_SIZE] = {0};         // response buffer, will be used to temporarily store the response
    char * method, * uri, * version;                 // parsed values from command_line
    char * scheme, * host, * filename, * port;       // parsed values from GET or CONNECT

    // define address
    struct sockaddr_in local_address;
    struct sockaddr_in remote_address;
    struct sockaddr_in server_address;





    // socket
    s = socket( AF_INET, SOCK_STREAM, 0);

    // terminate if error
    if( s == -1 ) {
        perror("socket() failed");
        return 1;
    }

    
    // define address
    local_address.sin_family      = AF_INET;
    local_address.sin_port        = htons(PORT);
    local_address.sin_addr.s_addr = 0;

    if ( -1 == setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ) {
        perror("setsockopt() failed");
        return 1;
    }

    // bind
    if( -1 == bind(s, (struct sockaddr *) &local_address, sizeof(struct sockaddr_in)) ) {
        perror("bind() failed");
        return 1;
    }

    // listen
    if( -1 == listen(s, 10) ) {
        perror("listen() failed");
        return 1;
    }

    // initialize remote (client) address
    remote_address.sin_family       = AF_INET;
    remote_address.sin_port         = htons(0);
    remote_address.sin_addr.s_addr  = 0;


    int sockaddr_size = sizeof(struct sockaddr);

    while(1) {
        
        // accept
        s_double = accept(s, (struct sockaddr *) &remote_address, &sockaddr_size);


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
        printf("Method —————> %s\nURI —————> %s\nVersion —————> %s\n\n\n\n", method, uri, version);



        if( !strcmp(method, "GET") ) { // GET http://www.example.com/dir/file

            scheme = uri;

            // parse the URI addredd, by getting the host and the resource
            for(i = 0; uri[i] != ':' && uri[i]; i++)

            if (uri[i] == ':')      // null terminate
                uri[i++] = 0;
            else {                  // check correctness
                printf("Parsing error (expected ':').\n");
                exit(1);
            }

            if (uri[i] != '/' || uri[i + 1] != '/') {
                printf("Parsing error (expected '//').\n");
                exit(1);
            }

            i = i + 2;

            // save host
            host = uri + (++i);

            // find position where host finishes
            for(; uri[i] && uri[i] != '/'; i++);


            if (uri[i] == '/')      // null terminate
                uri[i++] = 0;
            else {                  // check correctness
                printf("Parsing error (expected '/').\n");
                exit(1);
            }

            // initialize filename
            filename = uri + i;


            // resolve host name
            printf("GET host=%s\n", host);
            struct hostent * remote = gethostbyname(host);

            // create socket to connect to the remote
            s_remote = socket( AF_INET, SOCK_STREAM, 0);

            // terminate if error
            if( s_remote == -1 ) {
                perror("socket() failed");
                return 1;
            }

            // set up remote server address
            server_address.sin_family       = AF_INET;
            server_address.sin_port         = htons(80);
            server_address.sin_addr.s_addr  = *(unsigned int*)(remote->h_addr);


            // connect to the remote server
            if( -1 == connect( s_remote, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in))) {
                perror("connect() failed");
                return 1;
            }

            // create request
            snprintf(request_buffer, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", filename, host);

            // write request
            write(s_remote, request_buffer, strlen(request_buffer));

            // reset buffer
            for(i = 0; i < BUFFER_SIZE; i++) request_buffer[i] = 0;

            // receive response
            while( t = read(s_remote, response_buffer, BUFFER_SIZE))
                write(s_double, response_buffer, t);

            // close socket
            close(s_remote);

        } else if( !strcmp(method, "CONNECT") ) { // CONNECT www.example.com:443 HTTP/1.1
            
            // parse host and port
            host = uri;

            // end of host
            for(i = 0; uri[i] != ':'; i++);

            // null-terminate
            uri[i++] = 0;

            // set port
            port = uri + i;

            // resolve host name
            printf("CONNECT host=%s\n", host);
            struct hostent * remote = gethostbyname(host);

            // terminate if error
            if (remote == NULL) {
                printf("gethostbyname() failed.\n");
                return 1;
            }

            // create socket to connect to the remote
            s_remote = socket( AF_INET, SOCK_STREAM, 0);

            // terminate if error
            if( s_remote == -1 ) {
                perror("socket() failed");
                return 1;
            }
            
            // setup remote address
            server_address.sin_family       = AF_INET;
            server_address.sin_port         = htons( (unsigned short) atoi(port) );
            server_address.sin_addr.s_addr  = * ( unsigned int* ) remote -> h_addr;

            // connect to the remote server
            if( -1 == connect( s_remote, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in))) {
                perror("connect() failed");
                return 1;
            }

            // create request
            snprintf(request_buffer, BUFFER_SIZE, "HTTP/1.1 200 Established\r\n\r\n");

            // write request
            write(s, request_buffer, strlen(request_buffer));

            // reset buffer
            for(i = 0; i < BUFFER_SIZE; i++) request_buffer[i] = 0;

            // s_remote is the socket to the server
            if( fork() ) { // parent

                // read response from server and forwards it to the client
                for(i = 0; t = read(s_remote, response_buffer + i, BUFFER_SIZE - i); i+=t) {
                    
                    // write response
                    write(s_double, response_buffer, strlen(response_buffer));

                    // reset buffer
                    for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;
                }

            } else { // child

                // receive response from client and forwards it to the server
                for(i = 0; t = read(s_double, response_buffer + i, BUFFER_SIZE - i); i+=t) {
                    
                    // write response
                    write(s_remote, response_buffer, strlen(response_buffer));

                    // reset buffer
                    for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;
                }

                close(s_remote);
                exit(1);

            }

        } else {
            // create response
            sprintf(response_buffer, "HTTP/1.1 501 Not Implemented\r\n\r\n");

            // send
            write(s_double, response_buffer, strlen(response_buffer));
        }

        // close socket and kill process
        close(s_double);
        exit(1);

    }

    return 0;

} // main
