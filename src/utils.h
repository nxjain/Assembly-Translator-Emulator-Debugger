/**
 * @file utils.h
 * @brief Declarations for any debugging functions like printBits
 * @details This header file declares any function that may be used for debugging like printBits.
 */
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define assert_msg(cond, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "Assertion %s failed: %s: line %d: ", #cond, __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
        abort(); \
    } \
} while (0)


extern void print_bits(const uint32_t num);
extern int64_t sign_extend(uint64_t bits, int bit_length);
extern int32_t *malloc_assert_num(int32_t input_number);
char* int_to_string(int num);
char* int_to_hex_string(int num);
int int_cmp(const void *a, const void *b);

#endif /* UTILS_H */
