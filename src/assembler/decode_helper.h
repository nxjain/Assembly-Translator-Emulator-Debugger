/**
 * @file decode_helper.h
 * @brief Header file containing constants, enums, and function declarations for opcode decoding helpers.
 *
 * This header file defines generic constants for opcode sizes, operand limits, specific instructions,
 * and utility macros. It also declares enums for opcode and operand indices, along with constant arrays
 * for opcode names. Function declarations include utilities to check labels, directives, opcodes,
 * and to read and interpret values from opcode segments.
 *
 * Constants include opcode-specific counts for various instruction types and minimum operand counts.
 * Macros provide shorthand operations and patterns related to data processing, transfers, and branch instructions.
 * External functions ensure proper usage and verification of assembler arguments.
 */

#ifndef DECODE_HELPER_H
#define DECODE_HELPER_H

#include <stdbool.h>

#include "../instructions.h"

// Generic constants used for removing magic numbers
#define OPCODE_SIZE 10
#define MAX_NUM_OPERANDS 5
#define is_halt_instruction(str) (strcmp(str, "and x0, x0, x0") == 0) 
#define HALT_INSTRUCTION 0x8a000000
#define ZERO_REGISTER "rzr"
#define is_zero_register(str) (strcmp(str + 1, "zr") == 0)
#define is_valid_register(str) (is_bit_mode_32(str)|| is_bit_mode_64(str) || is_zero_register(str))
#define ZERO_REGISTER_INDEX 31
#define FST_CHAR_INDEX 0
#define TERMINATION_CHARACTER '\0'

// For "is_opcode" function
#define NUM_ADD_SUB_INSTS 4
#define NUM_SHIFT_ALIAS_INSTS 5
#define NUM_MUL_INSTS 2
#define NUM_LOGIC_INSTS 8
#define NUM_LOGIC_N_INSTS 4
#define NUM_WIDE_MOVE_INSTS 3
#define NUM_LOAD_STORE_INSTS 2
#define NUM_BRANCH_INSTS 2 // ignore b.cond as this has a seperate check

// For number of opcode preconditions
#define MIN_ADD_SUB_OPERANDS 3
#define MIN_MUL_OPERANDS 4
#define MIN_LOGIC_OPERANDS 3
#define MIN_WIDE_MOVE_OPERANDS 2
#define MIN_LOAD_STORE_OPERANDS 2
#define MIN_BRANCH_OPERANDS 1

#define set_bit(bit) ((bit) = 1)

// Data Processing
#define is_bit_mode_32(str) (str[FST_CHAR_INDEX] == 'w')
#define is_bit_mode_64(str) (str[FST_CHAR_INDEX] == 'x')
#define is_hex_number(str) (strncmp(str, "0x", 2) == 0) 
#define is_immediate(str) ((str)[FST_CHAR_INDEX] == '#')
#define is_set_flags(opcode) (strlen(opcode) == 4) // strlen("adds") = 4 -> set flags, strlen("add") = 3 -> don't set flags
#define DIV_VAL_HW 16

// Data Transfers
#define OPEN_SQUARE_BRACKET '['
#define CLOSED_SQUARE_BRACKET ']'
#define is_pre_index(str) (str[strlen(str) - 1] == '!')
#define DT_REG_PATTERN 26

// Branch Instructions
#define BR_REG_PATTERN 543

// Enum for opcode indices
typedef enum {
    OP_ADD, OP_ADDS, OP_SUB, OP_SUBS, // add/sub
    OP_NEG, OP_NEGS,OP_CMN, OP_CMP,   // add/sub aliases
    OP_M_ADD, OP_M_SUB,               // mul
    OP_MUL, OP_M_NEG,                 // mul aliases
    OP_LSL, OP_LSR, OP_ASR, OP_ROR,   // shifts
    OP_AND, OP_ANDS, OP_BIC, OP_BICS, OP_EOR, OP_EON, OP_ORR, OP_ORN, // logic
    OP_TST, OP_MVN, OP_MOV,           // logic aliases
    OP_MOVN, OP_MOVK,OP_MOVZ,         // wide moves
    OP_LDR, OP_STR,                   // load and store
    OP_B, OP_BR, OP_B_COND,           // branch
    OP_EQ, OP_NE, OP_GE, OP_LT, OP_GT, OP_LE, OP_AL,  // branch_conditions
    NUM_OPCODES
} Opcode;

// Enum for operand indices
typedef enum {OPERAND_1, OPERAND_2, OPERAND_3, OPERAND_4, OPERAND_5} OperandIndex;

// Opcode Names
extern const char * const opcode_names[NUM_OPCODES];

//------------------------------------FUNCTION DECLARATIONS------------------------------------
// Check if the string is a valid label
extern bool is_label(char *str);

// Check if the string is a label with a literal value
extern bool is_label_literal(char *str);

// Check if the string is an assembly directive
extern bool is_directive(char *str);

// Check if the string is an integer assembly directive
extern bool is_int_directive(char *str);

// Read, interpret and return register value from the input opcode segment
extern unsigned int read_reg_value(char *opcode_segment);

// Read, interpret and return immediate value from the input opcode segment
extern unsigned int read_imm_value(char *opcode_segment);

// Read, interpret and return literal value from the input opcode segment
extern signed int read_literal(char *opcode_segment, bool *operand_is_label);

// Read, interpret and return shift type from the input opcode segment
extern unsigned int read_shift_type(char *opcode_segment);

// Read, interpret and return branch condition type from the input opcode segment
extern unsigned int read_branch_cond_type(char *opcode_segment);

// Check if opcode is in list of input opcodes, return true if found, false otherwise
extern bool is_opcode(const char *opcode, Opcode *opcodes, int num_opcodes);

// Assert if assemble function doesn't receive its required number of arguments
extern void assert_num_opcodes(char **operands, int num_required);

#endif /* DECODE_HELPER_H */
