/**
 * @file register.c
 * @brief Defines the getters and setters for registers.
 * @details This header file contains the function definitions for the getters and setters 
 *          for registers. These are required to ensure the difference 
 *          written to. It also stores the 
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "register.h"

// Size of an instruction in bytes.
#define INSTR_SIZE 4

// Defines the union for a general register
typedef union {
    uint64_t xn;
    uint32_t wn;
} GeneralRegister;

// Defines the union for the special registers
typedef struct {
    const uint64_t zero_register;
    uint64_t program_counter;
    uint64_t stack_pointer;
} SpecialRegisters;

//Global variables to store all registers.
static GeneralRegister gen_registers[NUM_REGISTERS];
static SpecialRegisters spec_registers = {0, 0, 0};

/**
 * @brief Initializes all general-purpose registers to zero.
 */
void init_register(void) {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        gen_registers[i].xn = 0;
    }
    set_spec_register(PROGRAM_COUNTER, 0);
}

/**
 * @brief Sets the value of a specified general-purpose register using the index number of the register.
 *
 * This function assigns a 64-bit value to the specified general-purpose register.
 * It first checks if the register number is valid. If the register number is equal to `NUM_REGISTERS`,
 * the function does nothing, as this represents the zero register which should not be modified.
 *
 * @param reg_num The register number to which the value will be assigned.
 * @param value The 64-bit value to be assigned to the register.
 *
 * @note The function exits the program with a failure status if the register number is invalid.
 */
void set_reg_value(uint32_t reg_num, uint64_t value) {
    if (reg_num == NUM_REGISTERS) { //zr
        return;
    }
    if (reg_num > NUM_REGISTERS){
        fprintf(stderr, "Error: Register %d does not exist.\n", reg_num);
        exit(EXIT_FAILURE); 
    }
    gen_registers[reg_num].xn = value;
}

/**
 * @brief Retrieves the 32-bit value from a specified general-purpose register.
 *
 * This function returns the 32-bit value stored in the specified general-purpose register.
 * If the register number is equal to `NUM_REGISTERS`, it returns the value of zero register.
 *
 * @param reg_num The register number from which to retrieve the 32-bit value.
 * @return The 32-bit value stored in the specified register.
 *
 * @note The function exits the program with a failure status if the register number is invalid.
 */
uint32_t get_reg_value_32(uint32_t reg_num){
    if (reg_num == NUM_REGISTERS){ // zr
        return spec_registers.zero_register;
    }

    if (reg_num > NUM_REGISTERS){
        fprintf(stderr, "Error: Register %d does not exist.\n", reg_num);
        exit(EXIT_FAILURE); 
    }

    return gen_registers[reg_num].wn;
}

/**
 * @brief Retrieves the 64-bit value from a specified general-purpose register.
 *
 * This function returns the 64-bit value stored in the specified general-purpose register.
 * If the register number is equal to `NUM_REGISTERS`, it returns the value of zero register.
 *
 * @param reg_num The register number from which to retrieve the 64-bit value.
 * @return The 64-bit value stored in the specified register.
 *
 * @note The function exits the program with a failure status if the register number is invalid.
 */
uint64_t get_reg_value_64(uint32_t reg_num){
    if (reg_num == NUM_REGISTERS){ // zr
        return spec_registers.zero_register;
    }

    if (reg_num > NUM_REGISTERS){
        fprintf(stderr, "Error: Register %d does not exist.\n", reg_num);
        exit(EXIT_FAILURE); 
    }

    return gen_registers[reg_num].xn;
}

/**
 * @brief Retrieves the value of a specified special register.
 *
 * @param reg_type The type of the special register to retrieve.
 * @return The 64-bit value of the specified special register.
 *
 * @note The function exits the program with a failure status if the register type is invalid.
 */
uint64_t get_spec_register(SpecRegisterType reg_type){
    switch (reg_type){
        case ZERO_REGISTER:
            return spec_registers.zero_register;
        case PROGRAM_COUNTER:
            return spec_registers.program_counter;
        case STACK_POINTER:
            return spec_registers.stack_pointer;
        default:
            fprintf(stderr, "Error: special register does not exist\n"); // This should never be run (just in case)
            exit(EXIT_FAILURE); 
    }
}

/**
 * @brief Sets the value of a specified special register.
 *
 * @param reg_type The type of the special register to set.
 * @param value The 64-bit value to assign to the specified special register.
 *
 * @note The function exits the program with a failure status if an invalid register type is provided
 *       or if there is an attempt to write to the stack pointer register.
 */
void set_spec_register(SpecRegisterType reg_type, uint64_t value){
    switch (reg_type){
        case ZERO_REGISTER:
            break;
        case PROGRAM_COUNTER:
            spec_registers.program_counter = value;
            break;
        case STACK_POINTER:
            fprintf(stderr, "Error: Cannot write to stack pointer register\n");
            exit(EXIT_FAILURE); 
        default:
            fprintf(stderr, "Error: Unknown register type\n"); // This should never be run (just in case)
            exit(EXIT_FAILURE); 
    }
}

/**
 * @brief Increases the program counter by a specified offset.
 * @param offset The offset to add to the program counter. This value can be positive or negative.
 */
void increase_pc(int64_t offset) {
    spec_registers.program_counter += offset;
}

/**
 * @brief Increments the program counter by the instruction size.
 */
void increment_pc(void) {
    spec_registers.program_counter += INSTR_SIZE;
}

/**
 * @brief Prints the contents of all registers to the specified output file, for the final ".out" file.
 * 
 * This is used when printing out the final output in the ".out" file, as specified by the specification
 *
 * @param output_file Pointer to the file where the memory contents will be printed.
 */
void print_registers(FILE* output_file) {
    fprintf(output_file, "Registers:\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        fprintf(output_file, "X%02d    = %016lx\n", i, get_reg_value_64(i));
    }

    fprintf(output_file, "PC     = %016lx\n", get_spec_register(PROGRAM_COUNTER));
}
