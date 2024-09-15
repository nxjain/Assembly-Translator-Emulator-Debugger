/**
 * @file cpu.h
 * @brief Declarations for all enums, structs, constants and functions used in running the CPU.
 * @details This header file declares any function that is used for running the CPU - init_cpu
 *          run_cpu, and print_cpu().
 */
#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include "../instructions.h"
#include "../ADTs/hashmap.h"

// Instruction size in bytes
#define INSTR_SIZE 4
#define HALT_INSTRUCTION 0x8a000000
// Most significant bit for 32-bit integer
#define MSB_32_BIT (1UL << 31)
// Constant for shifting by 32
#define CONST_32 32
// Most significant bit for 64-bit integer
#define MSB_64_BIT (1ULL << 63)
// Constant for shifting by 64
#define CONST_64 64

// Processor state flags structure
typedef struct {
    bool negative_flag;   // Flag indicating negative result
    bool zero_flag;       // Flag indicating zero result
    bool carry_flag;      // Flag indicating carry in arithmetic operations
    bool overflow_flag;   // Flag indicating arithmetic overflow
} processor_state;

// Extern function declarations
extern void init_cpu(const char* input_file_path);   // Initialize CPU with instructions from file
extern void run_cpu(void);                           // Run CPU simulation
extern bool step_instruction();
extern void print_cpu(const char* output_file_path); // Print CPU state to file or stdout
extern processor_state get_pstate();
#endif
