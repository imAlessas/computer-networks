#include <stdio.h>
#include <sys/socket.h>             // socket
#include <errno.h>                  // errno
#include <arpa/inet.h>              // htons
#include <unistd.h>                 // write
#include <string.h>                 // strlen, strcmp
#include <stdlib.h>                 // atoi
#include <time.h> 



#define CAHCE_PATH "./cache/"
#define RES "index.html"
#define RESPONSE_SIZE (100 * 1024)



char hbuf[10000];

struct headers{
    char * n; 
    char * v; 
} h[100];     


int month2int(char *month) {
    if (!strcmp(month, "Jan")) return 1;
    else if (!strcmp(month, "Feb")) return 2;
    else if (!strcmp(month, "Mar")) return 3;
    else if (!strcmp(month, "Apr")) return 4;
    else if (!strcmp(month, "May")) return 5;
    else if (!strcmp(month, "Jun")) return 6;
    else if (!strcmp(month, "Jul")) return 7;
    else if (!strcmp(month, "Aug")) return 8;
    else if (!strcmp(month, "Sep")) return 9;
    else if (!strcmp(month, "Oct")) return 10;
    else if (!strcmp(month, "Nov")) return 11;
    else if (!strcmp(month, "Dec")) return 12;

    return -1;
}

int day2sunday(char *day) {
    if (!strcmp(day, "Sun")) return 0;
    else if (!strcmp(day, "Mon")) return 1;
    else if (!strcmp(day, "Tue")) return 2;
    else if (!strcmp(day, "Wed")) return 3;
    else if (!strcmp(day, "Thu")) return 4;
    else if (!strcmp(day, "Fri")) return 5;
    else if (!strcmp(day, "Sat")) return 6;

    return -1;
}


struct tm get_tm_date(char * date_string) {

    char * date_buffer = date_string;
    struct tm date = {0};
    int i;

    // skip name of day
    for(i = 0; date_string[i] != ','; i++);
    date_string[i] = 0;
    date.tm_wday = day2sunday(date_buffer);
    date_string[++i] = 0;


    // extract day
    date_buffer = date_buffer + i + 1;
    for(++i; date_string[i] != ' '; i++);
    date_string[i++] = 0;

    date.tm_mday = atoi(date_buffer);


    // extract month
    date_buffer = date_string + i;
    for(; date_string[i] != ' '; i++);
    date_string[i++] = 0;

    date.tm_mon = month2int(date_buffer) - 1;


    // extract year
    date_buffer = date_string + i;
    for(; date_string[i] != ' '; i++);
    date_string[i++] = 0;

    date.tm_year = atoi(date_buffer) - 1900;


    // extract hour
    date_buffer = date_string + i;
    for(; date_string[i] != ':'; i++);
    date_string[i++] = 0;

    date.tm_hour = atoi(date_buffer);


    // extract min
    date_buffer = date_string + i;
    for(; date_string[i] != ':'; i++);
    date_string[i++] = 0;

    date.tm_min = atoi(date_buffer);


    // extract sec
    date_buffer = date_string + i;
    for(; date_string[i] != ' '; i++);
    date_string[i++] = 0;

    date.tm_sec = atoi(date_buffer);

    // set the remaining fields
    date.tm_isdst = -1;  // Let mktime determine if DST is in effect

    char date_str[100];
    strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", &date);
    printf("Converted Date: %s\n", date_str);

    return date;
}


int main(){

    // local varibles
    struct sockaddr_in server_addr;     // server address
    int s;                              // socket
    int t;                              // temporary
    unsigned char * p;                  // ip address piointer
    int i, j;
    char * statusline;
    char * expires_string = NULL;
    char file_path[1024] = {0};
    char file_name[] = RES;
    char CRLF[] = "\r\n";
    char response[RESPONSE_SIZE] = {0};
    char cache_date_string[200] = {0};




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
    char * request = "HEAD / HTTP/1.1\r\n\r\n";
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
            
            j++;
            h[j].n = &hbuf[i + 1];
        }

        // end of name
        if( (hbuf[i] == ':') && (h[j].v == NULL) ){

            h[j].v = &hbuf[i + 1] + 1;
            hbuf[i] = 0;
        }
    }

    // print headers
    for(i = 0; i < j; i++)
        printf("%s —————> %s\n", h[i].n, h[i].v);
    printf("\n\n");


    // get last modifed
    for(i = 0; i < j; i++)
        if( !strcmp( h[i].n , "Expires" ))
            expires_string = h[i].v;
    
    // expires_string = "Thu, 01 Jul 2024 09:51:12 GMT";
    printf("Expires: %s\n", expires_string);

    if( !expires_string ) {
        printf("No 'Expires' header found\n");
        return 1;
    }


    struct tm real_expires_date = get_tm_date(expires_string);



    /* CHECK INSIDE CACHE */



    // substitute '/' with '_'
    for( i = 0; i < strlen(file_name); i++)
        if( file_name[i] == '/' )
            file_name[i] = '_';

    // save file cached file path
    snprintf(file_path, 1024, "%s%s", CAHCE_PATH, file_name);

    printf("Cache file path: %s\n", file_path);


    // opens file
    FILE * cache_file = fopen(file_path, "r");

    if ( !cache_file ) {
        
        printf("File '%s' does NOT exist.\n", file_path);

        // send request
        char * request = "GET / HTTP/1.0\r\n\r\n";
        write(s, request, strlen(request));

        // read and ignore header
        for( i = 0; read(s, hbuf + i, 1); i++ ){

            // end of line
            if( hbuf[i - 1] == '\r' && hbuf[i] == '\n'){
                
                hbuf[i - 1] = 0;
            
                if( !( h[j].n[0] ) )
                    break;
                
                j++;
                h[j].n = &hbuf[i + 1];
            }

            // end of name
            if( (hbuf[i] == ':') && (h[j].v == NULL) ){

                h[j].v = &hbuf[i + 1] + 1;
                hbuf[i] = 0;
            }
        }

        // if file does not exist, create it

        // opens the file to append
        cache_file = fopen(file_path, "w");

        // write in the file
        char date_str[100];
        strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", &real_expires_date);
        fprintf(cache_file, "%s\n", date_str);
        
        // add CRLF
        fwrite(&CRLF, strlen(CRLF), 1, cache_file);

        // write response file
        for ( i = 0; t = read(s, response + i, RESPONSE_SIZE - 1 - i); i += t );
        fprintf(cache_file, "%s", response);
        printf("\n\n\n\n\n%s\n\n", response);

        fclose(cache_file);

        return 0;

    } // cache file does not exist


    /* if cache file exist check date*/
    printf("File '%s' EXISTS.\n", file_path);

    fgets(cache_date_string, 200, cache_file);
    printf("File Date: %s\n", cache_date_string);

    struct tm cache_date = get_tm_date(cache_date_string);

    // get current time
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    printf("NOW:   %d\n", mktime(current_time));
    printf("CACHE: %d\n", mktime(&cache_date));

    // check if it is expired
    if( difftime(mktime(current_time), mktime(&cache_date)) > 1) { // it is expired

        printf("EXPIRED.\n", file_path);

        // send request
        char * request = "GET / HTTP/1.0\r\n\r\n";
        write(s, request, strlen(request));

        // read and ignore header
        for( i = 0; read(s, hbuf + i, 1); i++ ){

            // end of line
            if( hbuf[i - 1] == '\r' && hbuf[i] == '\n'){
                
                hbuf[i - 1] = 0;
            
                if( !( h[j].n[0] ) )
                    break;
                
                j++;
                h[j].n = &hbuf[i + 1];
            }

            // end of name
            if( (hbuf[i] == ':') && (h[j].v == NULL) ){

                h[j].v = &hbuf[i + 1] + 1;
                hbuf[i] = 0;
            }
        }

        // opens the file to append
        cache_file = fopen(file_path, "w");

        // write in the file
        char date_str[100];
        strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S %Z", &real_expires_date);
        fprintf(cache_file, "%s\n", date_str);
        
        // add CRLF
        fwrite(&CRLF, strlen(CRLF), 1, cache_file);

        // write response file
        for ( i = 0; t = read(s, response + i, RESPONSE_SIZE - 1 - i); i += t );
        fprintf(cache_file, "%s", response);
        printf("\n\n\n\n\n%s\n\n", response);

        fclose(cache_file);

        return 0;

    }

    printf("NOT EXPIRED\n\n\n\n");


    while ( !feof(cache_file) ) {
        fread(response, 1024, 1, cache_file);
        printf("%s\n", response);
        for( i=0; i < 1024; i++) response[i] = 0;
    }


    return 0;

} // main
