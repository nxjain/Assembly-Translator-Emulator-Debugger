/**
 * @file utils.c
 * @brief Definitions for any utility functions like print_bits, sign_extend 
 * @details This source file declares any function that may be used more than once but doesn't belong to other files.
 */

#include "utils.h"
#include <assert.h>

/**
 * The function `print_bits` prints out a given number in binary form for debugging purposes.
 * 
 * @param num The function `print_bits` takes an unsigned 32-bit integer `num` as input and prints out
 * the binary representation of that number for debugging purposes.
 */
void print_bits(const uint32_t num) {
    unsigned char *b = (unsigned char*) &num;
    unsigned char byte;
    int i, j;
    
    // Loop through each byte of "num" (from the MSB to the LSB)
    for (i = sizeof(num) - 1; i >= 0; i--) {
        // Loop through each bit of the current byte (from the MSB to the LSB)
        for (j = 7; j >= 0; j--) {
            // Extract the j-th bit of the i-th byte
            byte = (b[i] >> j) & 1;

            printf("%u", byte);
        }
        // Print a space after every 8 bits for better readability
        printf(" ");
    }
    puts("");
}

/**
 * The function sign_extend extends the sign of a given bit sequence to a 64-bit signed integer.
 * 
 * @param bits The `bits` parameter represents the binary value that needs to be sign-extended.
 * @param bit_length The `bit_length` parameter represents the number of bits in the input `bits` that
 * are considered for sign extension.
 * 
 * @return The function `sign_extend` takes an unsigned 64-bit integer `bits` and a signed integer
 * `bit_length` as input parameters. It checks the sign bit of `bits` based on the `bit_length`
 * provided. If the sign bit is 1 (indicating a negative number), it sign extends the number by
 * performing a bitwise OR operation with a mask that extends the sign bit to
 */
int64_t sign_extend(uint64_t bits, int bit_length) {
    if ((bits >> (bit_length - 1)) == 1) { // sign bit is 1
        return (int64_t) ((0xFFFFFFFFFFFFFFFF ^ ((1 << (bit_length)) - 1)) | bits);
    } else {
        return (int64_t) bits;
    }
}

int32_t *malloc_assert_num(int32_t input_number){
    int32_t* number_ptr = malloc(sizeof(int32_t));
    assert_msg(number_ptr != NULL, "MEMORY ALLOCATION ERROR: malloc failed\n");
    *number_ptr = input_number;
    return number_ptr;
}

char* int_to_string(int num) {
    // Step 1: Calculate the buffer size needed
    int size = snprintf(NULL, 0, "%d", num) + 1; // +1 for the null terminator

    // Step 2: Allocate memory dynamically
    char *str = (char *)malloc(size);
    if (str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Step 3: Convert the integer to string
    sprintf(str, "%d", num);

    return str;
}

char* int_to_hex_string(int num) {
    // Step 1: Calculate the buffer size needed
    int size = snprintf(NULL, 0, "%x", num) + 1; // +1 for the null terminator

    // Step 2: Allocate memory dynamically
    char *str = (char *)malloc(size);
    if (str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Step 3: Convert the integer to string
    sprintf(str, "%x", num);

    return str;
}

int int_cmp(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}
