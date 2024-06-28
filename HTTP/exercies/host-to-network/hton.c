#include <stdio.h>
#include <stdlib.h>





/**
 * @brief Determines the endianness of the current system.
 *
 * This function checks the endianness of the current system by inspecting the
 * least significant byte of an integer. If the least significant byte is 0,
 * the system is big-endian; otherwise, it is little-endian.
 *
 * @return 1 if the system is big-endian, 0 if the system is little-endian.
 */
char endianness(void) {
    // create an integer with the value 1 (0x00000001 in binary)
    int t = 1;

    // get a pointer to the least significant byte of the integer
    // if the system is big-endian, the pointer will point to the most significant byte (0x00)
    // if the system is little-endian, the pointer will point to the least significant byte (0x01)
    return ! (char *) &t;

} // endianness



/**
 * @brief Reverses the endianness of a long integer.
 *
 * This function takes a long integer and swaps the bytes to reverse the endianness.
 * If the input value is in little-endian format, the function will convert it to
 * big-endian format, and vice versa.
 *
 * @param n The long integer to be converted.
 * @return The long integer with the endianness reversed.
 */
long change_endian(long n) {
    // create a copy of the input value
    long n_reverse = n;

    // get a pointer to the individual bytes of the long integer
    unsigned char* cell = (unsigned char*)&n_reverse;

    // swap the bytes of the long integer using a simple XOR swap algorithm
    for (int i = 0; i < sizeof(long) / 2; i++) {
        // swap the i-th byte with the (sizeof(long) - 1 - i)-th byte
        cell[i] = cell[i] ^ cell[sizeof(long) - 1 - i];
        cell[sizeof(long) - 1 - i] = cell[i] ^ cell[sizeof(long) - 1 - i];
        cell[i] = cell[i] ^ cell[sizeof(long) - 1 - i];
    }

    return n_reverse;

} // change_endian




/**
 * @brief Converts a long integer from host byte order to network byte order.
 *
 * This function takes a long integer `n` in host byte order (the native byte order
 * of the machine) and returns the same value in network byte order (big-endian).
 * This is useful when sending or receiving data over a network, where the byte
 * order may be different from the host machine.
 *
 * @param n The long integer to be converted.
 * @return The long integer in network byte order.
 */
long hton(long n) {
    // check the endianness of the current system
    if (endianness()) 
        // if the system is big-endian, the host byte order is already in network byte order
        // so, we can simply return the input value `n` as is
        return n;

    // if the system is little-endian, we need to convert the byte order
    // the `change_endian()` function can be used for this purpose
    return change_endian(n);
    
} // hton





int main(int argc, char* argv[]) {
    printf("\n");

    // check if the correct number of arguments is provided
    if (argc > 2) {
        printf("Error: too many arguments.\n");
        return 1;
    }

    if (argc < 2) {
        printf("Error: too few arguments.\n");
        return 1;
    }

    // extract the number from the command-line argument
    long n = atoi(argv[1]);
    printf("%d\n", n);

    // print the individual bytes of the original long integer
    unsigned char* cell = (unsigned char*)&n;
    for (int i = 0; i < sizeof(long); i++)
        printf("  %d\n", cell[i]);


    // convert the long integer from host byte order to network byte order
    long n_reverse = hton(n);
    printf("\n\n%d\n", n_reverse);

    // print the individual bytes of the converted long integer
    cell = (unsigned char*)&n_reverse;
    for (int i = 0; i < sizeof(long); i++)
        printf("  %d\n", cell[i]);


    printf("\n");
    return 0;

} // main
