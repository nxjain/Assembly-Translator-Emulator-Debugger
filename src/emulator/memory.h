/**
 * @file memory.h
 * @brief Header file for memory management functions.
 *
 * This file contains the declarations of functions for managing memory, 
 * including initializing memory, loading instructions, accessing and modifying 
 * memory contents, and printing memory states.
 *
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include "../ADTs/darray.h"

#define NUM_OF_MEMORY_ADDRESS (1 << 21)

typedef uint32_t word;
typedef uint64_t double_word;

// Initializes memory to zero.
extern void init_memory(void); 

// Loads instructions from a file into memory using a file path.
extern void load_instructions_to_memory(FILE* input_file);

// Loads instructions from a file into memory using an array.
extern void load_instructions_to_memory_array(DArray* input_data);

// Retrieves a word from the specified memory address.
extern word get_word(uint32_t address);
// Sets a word at the specified memory address.
extern void set_word(uint32_t address, word data);

// Retrieves a double word from the specified memory address.
extern double_word get_double_word(uint32_t address);
// Sets a double word at the specified memory address.
extern void set_double_word(uint32_t address, double_word data);

// Prints non-zero memory contents to the specified output file.
extern void print_memory(FILE* output_file);

#endif /* MEMORY_H */
