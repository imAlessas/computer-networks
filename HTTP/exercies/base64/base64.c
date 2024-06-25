#include <stdio.h>
#include <string.h>
#include <unistd.h>


// define the Base64 alphabet
static const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


/**
 * @brief perform Base64 encoding on a given buffer.
 *
 * @param buffer the input buffer to be encoded.
 * @param coded the output buffer to store the encoded data.
 */
void base64(char* buffer, char* coded) {
    int symbols = strlen(buffer);

    // handle the case when the input buffer has 3 bytes
    if (symbols == 3) {
        coded[0] = base64_alphabet[(buffer[0] & 0xFC) / 4];
        coded[1] = base64_alphabet[((buffer[0] & 0x3) * 16) | ((buffer[1] & 0xF0) / 16)];
        coded[2] = base64_alphabet[((buffer[1] & 0xF) * 4) | ((buffer[2] & 0xC0) / 64)];
        coded[3] = base64_alphabet[(buffer[2] & 0x3F)];
    }

    // handle the case when the input buffer has 2 bytes
    if (symbols == 2) {
        coded[0] = base64_alphabet[(buffer[0] & 0xFC) / 4];
        coded[1] = base64_alphabet[((buffer[0] & 0x3) * 16) | ((buffer[1] & 0xF0) / 16)];
        coded[2] = base64_alphabet[((buffer[1] & 0xF) * 4)];
        coded[3] = '=';
    }

    // handle the case when the input buffer has 1 byte
    if (symbols == 1) {
        coded[0] = base64_alphabet[(buffer[0] & 0xFC) / 4];
        coded[1] = base64_alphabet[((buffer[0] & 0x3) * 16)];
        coded[2] = '=';
        coded[3] = '=';
    }

    // null-terminate the coded buffer
    coded[4] = 0;
}





int main(int argc, char* argv[]) {
    printf("\n");

    // check if the correct number of arguments is provided
    if (argc > 2) {
        printf("Error: too many arguments.");
        return 1;
    }

    if (argc < 2) {
        printf("Error: too few arguments.");
        return 1;
    }

    FILE* input_file, * output_file;
    char output_file_name[100], buffer[3], coded[5];
    size_t bytes_read;

    // open the input file
    input_file = fopen(argv[1], "rt");

    // terminate if there's an error opening the file
    if (input_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    printf("Processing file:\n   \033[36m%s\033[0m\n", argv[1]);


    // define the output file name
    sprintf(output_file_name, "base64_%s", argv[1]);

    // create/empty the output file
    fclose(fopen(output_file_name, "w"));

    // open the output file in append mode
    output_file = fopen(output_file_name, "a");

    printf("\nCreated file:\n   \033[1;35m%s\033[0m\n", output_file_name);

    printf("\n\033[1;38mBase64 coding:\033[0m\n");

    // read the input file in 3-byte chunks and encode them
    while ((bytes_read = fread(buffer, 1, 3, input_file)) > 0) {
        // null-terminate the buffer
        buffer[bytes_read] = 0;

        // perform Base64 encoding on the buffer
        base64(buffer, coded);

        // print the encoded data
        printf("%s", coded);

        // write the encoded data to the output file
        fwrite(coded, sizeof(char), bytes_read + 1, output_file);
    }

    printf("\n\n");

    // close the input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;

} // main
