/**
 * @file memory.c
 * @brief Memory management and instruction loading module.
 *
 * This file contains functions for initializing memory, loading instructions from a file into memory,
 * accessing and modifying words and double words in memory, and printing non-zero memory contents.
 *
 * Functions:
 * - init_memory: Initializes memory by setting all addresses to zero.
 * - load_instructions_to_memory: Loads instructions from a file into memory.
 * - get_word: Retrieves a word from a specified memory address.
 * - set_word: Sets a word at a specified memory address.
 * - get_double_word: Retrieves a double word from a specified memory address.
 * - set_double_word: Sets a double word at a specified memory address.
 * - print_memory: Prints non-zero memory contents to a specified output file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

// Size of an instruction in bytes.
#define INSTR_SIZE 4

// Array representing memory.
static uint8_t mem[NUM_OF_MEMORY_ADDRESS];

// Initializes memory by setting all addresses to zero.
void init_memory(void) {
    for (int i = 0; i < NUM_OF_MEMORY_ADDRESS; i++) {
        mem[i] = 0;
    }
}

/**
 * @brief Loads instructions from a file into memory.
 *
 * This function reads the contents of the specified input file and loads the instructions
 * into the memory array. The size of each instruction is defined by `INSTR_SIZE`. The
 * function first determines the size of the input file to ensure it does not exceed the
 * available memory space. It then reads the instructions from the file and writes them
 * into the memory array.
 *
 * @param input_file Pointer to the input file containing instructions to be loaded.
 *
 * @note The function exits the program with a failure status if:
 * - The input file size exceeds the memory capacity (`NUM_OF_MEMORY_ADDRESS`).
 * - An error occurs while reading from the input file.
 */
void load_instructions_to_memory(FILE* input_file) {
    //determine the file size
    fseek(input_file, 0, SEEK_END);
    const int file_size = ftell(input_file);
    rewind(input_file);

    const int num_of_instructions = file_size / INSTR_SIZE; 

    if (file_size > NUM_OF_MEMORY_ADDRESS) {
        perror("Input file size too large for memory\n");
        exit(EXIT_FAILURE);
    }
    
    // Write the content of the file to memory array
    for (int i = 0; i < num_of_instructions; i++) {
        int result = fread(mem + i * INSTR_SIZE, INSTR_SIZE, 1, input_file);

        // Error if reads more or less than 1 element
        if (result != 1) {
            perror("Failed to read from file input file\n");
            fclose(input_file);
            exit(EXIT_FAILURE);
        }
    }
}

void load_instructions_to_memory_array(DArray* input_data) {
    // Calculate the number of instructions
    int data_size = darray_length(input_data);
    const int num_of_instructions = data_size;

    // Check if the data size exceeds memory size
    if (data_size > NUM_OF_MEMORY_ADDRESS) {
        perror("Input data size too large for memory\n");
        exit(EXIT_FAILURE);
    }

    // Write the content of the input_data array to memory array
    for (int i = 0; i < num_of_instructions; i++) {
        // Copy the instruction to memory
        memcpy(mem + i * INSTR_SIZE, darray_get(input_data, i), INSTR_SIZE);
    }
}

/**
 * @brief Retrieves a word from the specified memory address.
 *
 * @param address The memory address from which to retrieve the word.
 * @return The 32-bit word read from the specified memory address.
 *
 * @note The function exits the program with a failure status if the address is out of bounds.
 */
word get_word(uint32_t address) {
    if (address > NUM_OF_MEMORY_ADDRESS - sizeof(word)) {
        fprintf(stderr, "Out of bounds trying to access word from memory address 0x%x\n", address);
        exit(EXIT_FAILURE);
    }

    word data;
    memcpy(&data, mem + address, sizeof(word));
    return data;
}

/**
 * @brief Retrieves a word (32-bit) from the specified memory address.
 *
 * @param address The memory address from which to retrieve the word.
 * @return The 32-bit word read from the specified memory address.
 *
 * @note The function exits the program with a failure status if the address is out of bounds.
 */
void set_word(uint32_t address, word data) {
    if (address > NUM_OF_MEMORY_ADDRESS - sizeof(word)) {
        fprintf(stderr, "Out of bounds trying to access word from memory address 0x%x\n", address);
        exit(EXIT_FAILURE);
    }

    memcpy(mem + address, &data, sizeof(word));
}

/**
 * @brief Retrieves a double word (64-bit) from the specified memory address.
 *
 * @param address The memory address from which to retrieve the double word.
 * @return The 64-bit double word read from the specified memory address.
 *
 * @note The function exits the program with a failure status if the address is out of bounds.
 */
double_word get_double_word(uint32_t address) {
    if (address > NUM_OF_MEMORY_ADDRESS - sizeof(double_word)) {
        fprintf(stderr, "Out of bounds trying to access double word from memory address 0x%x\n", address);
        exit(EXIT_FAILURE);
    }

    double_word data;
    memcpy(&data, mem + address, sizeof(double_word));
    return data;
}

/**
 * @brief Sets a double word (64-bit) at the specified memory address.
 *
 * @param address The memory address at which to set the double word.
 * @param data The 64-bit double word to write to the specified memory address.
 * 
 * @note The function exits the program with a failure status if the address is out of bounds.
 */
void set_double_word(uint32_t address, double_word data) {
    if (address > NUM_OF_MEMORY_ADDRESS - sizeof(double_word)) {
        fprintf(stderr, "Out of bounds trying to access double word from memory address 0x%x\n", address);
        exit(EXIT_FAILURE);
    }

    memcpy(mem + address, &data, sizeof(double_word));
}

/**
 * @brief Prints the non-zero memory contents to the specified output file.
 *
 * This is used when printing out the final output in the ".out" file, as specified by the specification
 *
 * @param output_file Pointer to the file where the memory contents will be printed.
 */
void print_memory(FILE* output_file) {
    fprintf(output_file, "Non-Zero Memory:\n");
    for (int i = 0; i < NUM_OF_MEMORY_ADDRESS; i += 4) {
        if (get_word(i) != 0) {
            fprintf(output_file, "0x%08x: %08x\n", i, get_word(i));
        }
    }
}
