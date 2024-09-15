/**
 * @file decode_helper.c
 * @brief Helper functions for decoding assembly instructions.
 *
 * This file contains utility functions used for decoding assembly instructions
 * into their corresponding machine code representations. It includes functions
 * for identifying labels, directives, opcodes, registers, immediate values, shift types,
 * branch conditions, and validating opcode presence.
 * 
 * Dependencies:
 * - decode_helper.h: Header file defining the interfaces for decode helper functions.
 * - ../utils.h: Utility functions and macros.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "decode_helper.h"
#include "../utils.h"

// Branch Instructions
#define IS_LABEL ':'

#define IS_DIRECTIVE '.'
#define IS_INT_DIRECTIVE ".int"

// All the strings required for identifying instructions:
const char * const opcode_names[] = {
    [OP_ADD]  = "add", [OP_ADDS] = "adds", [OP_SUB]  = "sub", [OP_SUBS] = "subs",
    [OP_NEG]  = "neg", [OP_NEGS] = "negs", [OP_CMN]  = "cmn", [OP_CMP]  = "cmp",
    [OP_M_ADD] = "madd", [OP_M_SUB] = "msub",
    [OP_MUL]  = "mul", [OP_M_NEG] = "mneg",
    [OP_LSL]  = "lsl", [OP_LSR]  = "lsr", [OP_ASR]  = "asr", [OP_ROR]  = "ror",
    [OP_AND]  = "and", [OP_ANDS] = "ands", [OP_BIC]  = "bic", [OP_BICS] = "bics",
    [OP_EOR]  = "eor", [OP_EON]  = "eon", [OP_ORR]  = "orr", [OP_ORN]  = "orn",
    [OP_TST]  = "tst", [OP_MVN]  = "mvn", [OP_MOV]  = "mov",
    [OP_MOVN] = "movn", [OP_MOVK] = "movk", [OP_MOVZ] = "movz",
    [OP_LDR] = "ldr", [OP_STR] = "str",
    [OP_B] = "b", [OP_BR] = "br", [OP_B_COND] = "b.",
    [OP_EQ]   = "eq", [OP_NE] = "ne", [OP_GE] = "ge", [OP_LT] = "lt", [OP_GT] = "gt", [OP_LE] = "le", [OP_AL]   = "al"
};

// ------------------------------------ASSEMBLE HELPER FUNCS:-----------------------------------

/**
 * @brief Checks if a string is a label.
 * 
 * Checks if the last character of the string is a label symbol (':').
 * 
 * @param str String to check.
 * @return true if the string is a label, false otherwise.
 */
bool is_label(char *str){
    assert_msg(str != NULL, "String can not be NULL\n");
    return str[strlen(str) - 1] == IS_LABEL;
}

/**
 * @brief Checks if a string is a valid label literal.
 * 
 * Checks if the string matches the format of a valid label:
 * - Starts with [a-zA-Z_\.]
 * - Followed by [a-zA-Z0-9$_\.]*
 * 
 * @param str String to check.
 * @return true if the string is a valid label literal, false otherwise.
 */
bool is_label_literal(char *str){
    assert_msg(str != NULL, "String can not be NULL\n");
    // Check if the first character matches [a-zA-Z_\.]
    if (!(isalpha(*str) || *str == '_' || *str == '.')) {
        return false;
    }
    str++;

    // Check if the rest of the string matches ([a-zA-Z0-9$_\.])*
    while (*str) {
        if (!(isalnum(*str) || *str == '$' || *str == '_' || *str == '.')) {
            return false;
        }
        str++;
    }

    return true;
}

/**
 * @brief Checks if a string is a directive.
 * 
 * Checks if the string starts with a directive symbol ('.').
 * 
 * @param str String to check.
 * @return true if the string is a directive, false otherwise.
 */
bool is_directive(char *str) {
    assert_msg(str != NULL, "String can not be NULL\n");
    return str[0] == IS_DIRECTIVE;
}

/**
 * @brief Checks if a string is an integer directive.
 * 
 * Checks if the string matches the integer directive ".int".
 * 
 * @param str String to check.
 * @return true if the string is ".int", false otherwise.
 */
bool is_int_directive(char *str) {
    assert_msg(str != NULL, "String can not be NULL\n");
    return strcmp(str, IS_INT_DIRECTIVE) == 0;
}

/**
 * @brief Reads and returns the register index from a string.
 * 
 * Parses the register index from the input string and returns it as an unsigned integer.
 * 
 * @param opcode_segment String containing the register name.
 * @return Unsigned integer value of the register index.
 */
unsigned int read_reg_value(char * opcode_segment){
    assert_msg(is_valid_register(opcode_segment), "The register passed into read_reg_value \"%s\" is invalid.", opcode_segment);
    if (is_zero_register(opcode_segment)){
        return ZERO_REGISTER_INDEX;
    }
    opcode_segment++; // remove "w" or "x" from the register name
    return atoi(opcode_segment);
}

/**
 * @brief Reads and returns an immediate value from a string.
 * 
 * Parses the immediate value from the input string and returns it as a uint32_t.
 * 
 * @param opcode_segment String containing the immediate value.
 * @return uint32_t value of the immediate.
 */
uint32_t read_imm_value(char * opcode_segment){
    if (is_immediate(opcode_segment)) {
        opcode_segment++; //To remove #
    }

    unsigned int immediate;
    // If immediate given as a hex number
    if (is_hex_number(opcode_segment)){
        immediate = strtol(opcode_segment, NULL, 16);
    } else{
        immediate = atoi(opcode_segment);
    }
    return immediate;
}

/**
 * @brief Reads and returns the shift type encoding from a string.
 * 
 * Parses the shift type from the input string and returns its corresponding encoding.
 * 
 * @param opcode_segment String containing the shift type.
 * @return Unsigned integer encoding of the shift type.
 */
unsigned int read_shift_type(char *opcode_segment){
    if (strcmp(opcode_segment, opcode_names[OP_LSL]) == 0){
        return ITP_LSL;
    }
    if (strcmp(opcode_segment, opcode_names[OP_LSR]) == 0){
        return ITP_LSR;
    }
    if (strcmp(opcode_segment, opcode_names[OP_ASR]) == 0){
        return ITP_ASR;
    }
    if (strcmp(opcode_segment, opcode_names[OP_ROR]) == 0){
        return ITP_ROR;
    }
    fprintf(stderr, "ERROR: Unrecognised shift name included: %s\n", opcode_segment);
    exit(EXIT_FAILURE);
}

/**
 * @brief Reads and returns the branch condition encoding from a string.
 * 
 * Parses the branch condition from the input string and returns its corresponding encoding.
 * 
 * @param opcode_segment String containing the branch condition.
 * @return Unsigned integer encoding of the branch condition.
 */

unsigned int read_branch_cond_type(char *opcode_segment){
    assert_msg(strncmp(opcode_segment, opcode_names[OP_B_COND], 2) == 0, "The instruction passed in to read_branch_cond_type \"%s\" is not a branch condition instruction (no beginning \"br\")", opcode_segment);

    //"Remove the b."
    opcode_segment++;
    opcode_segment++;

    if (strcmp(opcode_segment, opcode_names[OP_EQ]) == 0) {
        return ITP_EQ;
    }
    if (strcmp(opcode_segment, opcode_names[OP_NE]) == 0) {
        return ITP_NE;
    }
    if (strcmp(opcode_segment, opcode_names[OP_GE]) == 0) {
        return ITP_GE;
    }
    if (strcmp(opcode_segment, opcode_names[OP_LT]) == 0) {
        return ITP_LT;
    }
    if (strcmp(opcode_segment, opcode_names[OP_GT]) == 0) {
        return ITP_GT;
    }
    if (strcmp(opcode_segment, opcode_names[OP_LE]) == 0) {
        return ITP_LE;
    }
    if (strcmp(opcode_segment, opcode_names[OP_AL]) == 0) {
        return ITP_AL;
    }

    fprintf(stderr, "ERROR: Unrecognised branch condition name included: %s\n", opcode_segment);
    exit(EXIT_FAILURE);
}

/**
 * @brief Checks if a given opcode is in a list of opcodes.
 * 
 * This function iterates through a list of `opcodes` and compares each with
 * the provided `opcode` string. It returns true if a match is found, indicating
 * that the `opcode` exists in the list; otherwise, it returns false.
 * 
 * @param opcode The opcode string to check.
 * @param opcodes Array of Opcode values (indices) to compare against.
 * @param num_opcodes Number of elements in the `opcodes` array.
 * @return true if `opcode` is found in `opcodes`, false otherwise.
 */
bool is_opcode(const char *opcode, Opcode *opcodes, int num_opcodes) {
    for (int i = 0; i < num_opcodes; i++) {
        if (strcmp(opcode, opcode_names[opcodes[i]]) == 0) {
            return true; // Found a match
        }
    }
    return false; // No match found
}

/**
 * @brief Asserts if the number of operands does not match the required number.
 * 
 * This function checks if the number of operands (`operands`) matches the required
 * number (`num_required`). If any of the first `num_required` operands are NULL,
 * it asserts an error message indicating that there are not enough arguments provided.
 * 
 * @param operands Array of operand strings to check.
 * @param num_required Number of operands required.
 */
void assert_num_opcodes(char **operands, int num_required){
    for (int i = 0; i < num_required; i++){
        assert_msg(operands[i] != NULL, "Not enough arguments - Number of required arguments: %d | Number of arguments: %d\n", num_required, i);
    }
}
