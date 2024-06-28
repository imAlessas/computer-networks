#include <stdio.h>  // printf, perror, fopen, fread, feof, fclose
#include <string.h> // strlen
#include <stdlib.h> // exit
#include <unistd.h> // read, write, fork
#include <sys/socket.h> // socket, bind, listen, accept
#include <arpa/inet.h>  // htons, sockaddr, sockaddr_in




// define structure
struct map{
    char * key;
    char * value;
}; // map



int main() {

    // constant values
    const long PORT = 56456;            // define the port


    // local valiable definition
    int tmp;                            // temporary variable
    int s, s_double;                    // socket
    int size_sockaddr;                  // length of struct sockaddr, will pass it as a reference in the accept()
    struct sockaddr_in server_address;  
    struct sockaddr_in client_address;
    struct map headers[100];            // contains key and their value
    char buffer[100 * 1024];            // 100 Kb generic buffer
    char * command_line;                // contains the first line of the request
    int i;                              // generic index
    int n;                              // generic variable, changes its meaning along the script
    int trans;
    char * method, * uri, * version;    // information to retrive from the command_line
    FILE * file;                        // will contain the requested URI





    // create the socket
    s = socket(AF_INET, SOCK_STREAM, 0);

    // check errors
    if (s == -1) {
        perror("socket() failed");
        return 1;
    }


    // setup the server address
    server_address.sin_family       = AF_INET;
    server_address.sin_port         = htons(PORT);
    server_address.sin_addr.s_addr  = 0;

    // bind the socket with the server
    tmp = bind(s, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in));

    // check errors
    if (tmp == -1) {
        perror("bind() failed");
        return 1;
    }


    // listen for connections
    tmp = listen(s, 5);

    // check errors
    if (tmp == -1) {
        perror("listen() failed");
        return 1;
    }



    // run the server

    size_sockaddr = sizeof(struct sockaddr);

    while(1) {
        
        // close if is still open
        close(s_double);

        // set transaction to 0
        trans = 0;

        // dequeue requests from backlog
        s_double = accept(s, (struct sockaddr *) &client_address, &size_sockaddr);


        // split the process
        if(fork())
            continue;



        // terminate if errors
        if(s_double == -1){
            perror("accept() failed");
            return 1;
        }


        /* process the request */

        command_line = headers[0].key = buffer;
        n = 0;

        // reads the headers
        for(i = 0; tmp = read(s_double, buffer + i, 1); i++){
            
            // end the key of the header
            if( buffer[i] == ':' && headers[n].value == NULL ) {
                
                // set te pointer to the value
                headers[n].value = &buffer[i + 1];
                
                // null terminate
                buffer[i] = 0;
            }

            // end of the line (end of the value)
            if( buffer[i - 1] == '\r' && buffer[i] == '\n' ) {
                
                // null terminate
                buffer[i - 1] = 0;

                // if the last header is null then, by definition, the Request-Header ends
                if( !( headers[n].key[0] ) )
                    break;

                // new line
                n++;

                // set the next key
                headers[n].key = &buffer[i + 1];

            }

        }



        // from command_line retrive method, uri, version
        method = command_line;
        for( i = 0; command_line[i] != ' '; i++);
        command_line[i++] = 0;

        uri = command_line + i;
        for(; command_line[i] != ' '; i++);
        command_line[i++] = 0;

        version = command_line + i;
        for(; command_line[i] != 0; i++);
        command_line[i++] = 0;

        for(int i = 0; i < n; i++)
            printf("%s ----> %s\n", headers[i].key, headers[i].value);

        printf("%s     Socket %d     Trans %d\n\n\n", uri, s_double, trans++);


        // opens URI
        file = fopen(uri + 1, "rw");

        // file does not exist
        if( file == NULL) {
            
            // create response
            sprintf(buffer, "HTTP/1.1 404 NOT FOUND\r\n\r\n"
                            "<html><h1>Could not find %s.<h1></html>", uri);
            
            // send response
            write(s_double, buffer, strlen(buffer));

        } else {
            
            // create response header
            sprintf(buffer, "HTTP/1.1 200 OK\r\n\r\n");

            // send response header
            write(s_double, buffer, strlen(buffer));

            // read and send the file
            while( !feof(file) ){
                
                // read 1024 * 1 bytes from the file and put inside the buffer
                fread(buffer, 1, 1024, file);
                
                // send the respons (partial)
                write(s_double, buffer, strlen(buffer));

            }

        }

        // close socket and kill process
        close(s_double);
        exit(1);

    }


    return 0;

} // main
