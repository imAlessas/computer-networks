#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>



static const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const char* input_array, size_t input_size) {
    // Allocate memory for the output string
    char* output = (char*)malloc(((input_size + 2) / 3 * 4 + 1) * sizeof(char));
    if (output == NULL) {
        return NULL;
    }

    // Perform the Base64 encoding
    size_t output_index = 0;
    for (size_t i = 0; i < input_size; i += 3) {
        // Encode the next 3 bytes
        unsigned char byte1 = (i < input_size) ? input_array[i] : 0;
        unsigned char byte2 = (i + 1 < input_size) ? input_array[i + 1] : 0;
        unsigned char byte3 = (i + 2 < input_size) ? input_array[i + 2] : 0;

        output[output_index++] = base64_alphabet[(byte1 >> 2) & 0x3F];
        output[output_index++] = base64_alphabet[((byte1 & 0x3) << 4) | ((byte2 >> 4) & 0xF)];
        output[output_index++] = base64_alphabet[((byte2 & 0xF) << 2) | ((byte3 >> 6) & 0x3)];
        output[output_index++] = base64_alphabet[byte3 & 0x3F];
    }

    // Handle the case when the input size is not a multiple of 3
    if (input_size % 3 == 1) {
        output[output_index - 2] = '=';
        output[output_index - 1] = '=';
    } else if (input_size % 3 == 2) {
        output[output_index - 1] = '=';
    }

    // Null-terminate the output string
    output[output_index] = '\0';

    return output;
}



struct char_map {
    char * key;
    char * value;
};




int main() {

    // constants
    const short PORT = 9834;
    const int BUFFER_SIZE = 1024;
    const char username[] = "imAlessas";
    const char password[] = "31415926";

    // local variables
    int s, s_double;                    // sockets
    char * command_line;                // first line of request
    struct char_map headers[100];       // headers
    char header_buffer[BUFFER_SIZE];    // header buffer, here there will be all the info from the header
    char response_buffer[BUFFER_SIZE];  // response buffer, will be used to temporarily store the response
    char * method, * uri, * version;    // parsed values from command_line
    int i;                              // generic index
    char * auth_value, * base64_cred;

    // define address
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;



    // socket
    s = socket( AF_INET, SOCK_STREAM, 0);

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
                headers[lines].value = &header_buffer[i + 1];

                // null-terminate
                header_buffer[i] = 0;

            }
        }

        // find Authorization
        for(i = 0; i < lines; i++)
            if(!strcmp(headers[i].key, "Authorization"))
                auth_value = headers[i].value;


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
            
            // before accessing an existing file it is needed to AUTHENTICATE
            if( !auth_value ) {
                
                snprintf(response_buffer, BUFFER_SIZE, "HTTP/1.1 401 UNAUTHORIZED\r\nWWW-Authenticate: Basic realm=\"Users\"\r\nConnection: close\r\n\r\n");

                if( -1 == write(s_double, response_buffer, strlen(response_buffer)) ){
                    perror("write() failed");
                    return 1;
                }


                fclose(file);
                close(s_double);

                continue;

            }


            for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;

            // extract base64_cred
            for(i = 1; auth_value[i] != ' '; i++);
            base64_cred = auth_value + i + 1;

            

            snprintf(response_buffer, BUFFER_SIZE, "%s:%s", username, password);

            if( strcmp( base64_cred, base64_encode(response_buffer, strlen(response_buffer)))){

                printf("base64_cred = %s (%d)\n", base64_cred, strlen(base64_cred));
                printf("base64_corr = %s\n", base64_encode(response_buffer, strlen(response_buffer)));

                snprintf(response_buffer, BUFFER_SIZE, "HTTP/1.1 401 UNAUTHORIZED\r\nWWW-Authenticate: Basic realm=\"Users\"\r\nConnection: close\r\n\r\n");

                if( -1 == write(s_double, response_buffer, strlen(response_buffer)) ){
                    perror("write() failed");
                    return 1;
                }
                
                
                fclose(file);
                close(s_double);

                continue;
            }

            for(i = 0; i < BUFFER_SIZE; i++) response_buffer[i] = 0;


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
