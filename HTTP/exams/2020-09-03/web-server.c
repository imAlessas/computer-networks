#include <stdio.h>  // printf, perror, fopen, fread, feof, fclose
#include <string.h> // strlen
#include <stdlib.h> // exit
#include <unistd.h> // read, write, fork
#include <sys/socket.h> // socket, bind, listen, accept
#include <arpa/inet.h>  // htons, sockaddr, sockaddr_in



struct char_map {
    char * key;
    char * value;
};



int main() {

    // constants
    const short PORT = 5686;
    const int BUFFER_SIZE = 1024;
    const char COOKIE_NAME[] = "contact";

    // local variables
    int s, s_double;                    // sockets
    char * command_line;                // first line of request
    struct char_map headers[100];       // headers
    char header_buffer[BUFFER_SIZE];    // header buffer, here there will be all the info from the header
    char response_buffer[BUFFER_SIZE];  // response buffer, will be used to temporarily store the response
    char * method, * uri, * version;    // parsed values from command_line
    int i;                              // generic index
    char * client_cookie_string;
    char * cookie_string;
    int client_cookie_value;
    char * client_cookie_name;

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
        
        
        
        // retrieve cookie value
        for(i = 0; i < lines; i++)
            if( !strcmp(headers[i].key, "Cookie") )
                client_cookie_string = headers[i].value;
        
        // if is not null
        if( client_cookie_string ) {

            // extract name and value of the Cookie
            client_cookie_name = client_cookie_string;
            for(i = 0; client_cookie_string[i] != '='; i++);
            client_cookie_string[i++] = 0;
            
            if(client_cookie_name)
                client_cookie_value = atoi(client_cookie_string + i);
        }
            
        

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
            // if the client goes in the contact.html AND (does not have cookie OR the cookie name is incorrect OR the cookie value is incorrect)
            if( !strcmp(uri, "/contact.html") && ( !client_cookie_name || strcmp(client_cookie_name, COOKIE_NAME) || client_cookie_value != 1)) {
                
                snprintf(response_buffer, BUFFER_SIZE, "HTTP/1.1 403 Forbidden\r\nConnection:close\r\n\r\n<html><h1>You need to access <a href=\"/index.html\">/index.html</a> before entering this page.</h1></html>");

                write(s_double, response_buffer, strlen(response_buffer));

                for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;

                // close everything
                fclose(file);
                close(s_double);
                exit(1);
            }


            // if client is in index.html AND (does not have cookie OR the cookie name is incorrect OR the cookie value is incorrect)
            if( !strcmp(uri, "/index.html") && (!client_cookie_name || strcmp(client_cookie_name, COOKIE_NAME) || client_cookie_value != 1)) {

                sprintf(response_buffer, "HTTP/1.1 200 OK\r\nSet-Cookie:%s=%d\r\n\r\n", COOKIE_NAME, 1);

            } else if (!strcmp(uri, "/contact.html") && !strcmp(client_cookie_name, COOKIE_NAME) && client_cookie_value == 1) {
                
                sprintf(response_buffer, "HTTP/1.1 200 OK\r\nSet-Cookie:%s=%d\r\n\r\n", COOKIE_NAME, 0);
            
            } else {

                sprintf(response_buffer, "HTTP/1.1 200 OK\r\n\r\n");

            }

            if( -1 == write(s_double, response_buffer, strlen(response_buffer)) ){
                perror("write() failed");
                return 1;
            }

            for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;


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