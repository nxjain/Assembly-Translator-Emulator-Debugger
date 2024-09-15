/**
 * @file symbol_table.c
 * @brief Implementation file for symbol table management in assembly instructions.
 *
 * This file defines functions for managing symbol tables used for labels and addresses
 * in assembly instructions. It includes functions for initialization, adding labels,
 * retrieving addresses, modifying instructions based on labels, and freeing allocated memory.
 */

#include "symbol_table.h"
#include "../utils.h"
#include "../ADTs/hashmap.h"
#include "../instructions.h"

#define INSTR_SIZE 4

static HashMap *labels;     // HashMap to store labels and their corresponding literal addresses
static HashMap *addresses;  // HashMap to store labels and lists of addresses where they are used

/**
 * @brief Initializes the symbol tables for labels and addresses.
 * 
 * Initializes two HashMaps:
 * - `labels`: Maps label strings to literal addresses (uint32_t *).
 * - `addresses`: Maps label strings to lists of addresses (DArray *).
 * Each HashMap is initialized with appropriate free functions for cleanup.
 */
void symbol_table_init() {
    labels = hashmap_init(free);
    addresses = hashmap_init(darray_free);
}

/**
 * @brief Modifies the instruction at a given address based on a label's literal address.
 * 
 * This function adjusts the branch offsets in instructions based on the difference between
 * the literal address of a label and the current instruction address.
 * 
 * @param instruction Pointer to the instruction to modify.
 * @param instruction_address Current instruction address.
 * @param literal_address Literal address of the label.
 */
static void modify_line(uint32_t *instruction, uint32_t instruction_address, uint32_t literal_address) {
    Instruction inst;
    inst.data = *instruction;
    int offset = (literal_address - instruction_address) / INSTR_SIZE;

    // Handle branch instructions:
    if (inst.gen_branch.op0 == ITP_BRANCH) {
        if (inst.branch_unconditional.id == ITP_BRANCH_UNCOND) {
            inst.branch_unconditional.simm26 = offset;
            *instruction = inst.data;
            return;
        }

        if (inst.branch_conditional.id == ITP_BRANCH_COND) {
            inst.branch_conditional.simm19 = offset;
            *instruction = inst.data;
            return;
        }
    }

    // Handle data transfer load literal instructions:
    if (inst.gen_dt.op0_1 == ITP_DT_1 && inst.gen_dt.op0_2 == ITP_DT_2 && inst.dt_load_literal.id == ITP_DT_LOAD_LITERAL) {
        inst.dt_load_literal.simm19 = offset;
        *instruction = inst.data;
        return;
    }

    // Print the instruction bits and exit on failure if the instruction shouldn't have a branch literal
    print_bits(*instruction);
    fprintf(stderr, "Instruction passed in is not meant to have a branch literal: %x\n", *instruction);
    exit(EXIT_FAILURE);
}

/**
 * @brief Adds a label with its literal address to the symbol tables and adjusts instructions referencing it.
 * 
 * If the label already exists in `labels`, it asserts an error due to multiple definitions.
 * If `addresses` contains the label, it modifies all instructions that reference it.
 * 
 * @param instructions Array of instructions.
 * @param literal_address Literal address of the label.
 * @param label Label string to add.
 */
void symbol_table_add_label(DArray *instructions, uint32_t literal_address, char *label) {
    // Ensure the label is not already defined:
    assert_msg(
        !hashmap_contains(labels, label), 
        "Multiple definitions of label in address %x and %x\n", 
        *(uint32_t *) hashmap_get(labels, label),
        literal_address
    );

    // Store the literal address in `labels`:
    uint32_t *address_copy = malloc(sizeof(uint32_t));
    assert_msg(address_copy != NULL, "Memory allocation failed\n");
    *address_copy = literal_address;
    hashmap_set(labels, label, address_copy);

    // If `addresses` does not contain the label, then we have not encountered any "b label" lines before, so we are done.
    if (!hashmap_contains(addresses, label)) {
        return;
    }

    // Modify instructions that reference the label:
    DArray *addresses_of_instructions = hashmap_get(addresses, label);
    int i = 0;
    uint32_t *instruction_address;
    while (darray_iterator(addresses_of_instructions, &i, (void **) &instruction_address)) {
        uint32_t *instruction = darray_get(instructions, (*instruction_address) / INSTR_SIZE);
        modify_line(instruction, *instruction_address, literal_address);
    }
}

/**
 * @brief Retrieves the relative address of a label from the current instruction address.
 * 
 * Calculates and returns the relative address (in instructions) between the current
 * instruction address and the literal address of the label.
 * 
 * @param instruction_address Current instruction address.
 * @param label Label string to retrieve the address for.
 * @return Relative address (difference in instructions) between the label and the current instruction address.
 */
int symbol_table_get_address(uint32_t instruction_address, char *label) {
    // Check if the label exists in `labels`
    if (hashmap_contains(labels, label)) {
        uint32_t *literal_address = hashmap_get(labels, label);
        return ((int) *literal_address - (int) instruction_address) / INSTR_SIZE;
    }
    // Make space for the new address:
    uint32_t *copy = malloc(sizeof(uint32_t));
    assert_msg(copy != NULL, "Memory allocation failed\n");
    *copy = instruction_address;

    // If the label doesn't exist in `labels`, store the instruction address at the end of the dynamic array of the label in `addresses`
    if (hashmap_contains(addresses, label)) {
        DArray *addresses_of_instructions = hashmap_get(addresses, label);
        darray_add(addresses_of_instructions, copy);
        return 0; // Leave the address part unchanged until we change it later once we know the address of the label.
    }

    // Initialize a new DArray for the label and store the instruction address
    DArray *addresses_of_instructions = darray_init(free);
    darray_add(addresses_of_instructions, copy);

    // Add label key and DArray value to addresses hashmap
    hashmap_set(addresses, label, addresses_of_instructions);
    return 0;
}

/**
 * @brief Frees the memory allocated for symbol tables (`labels` and `addresses`).
 * 
 * Frees the memory allocated for `labels` and `addresses` HashMaps,
 * including the memory allocated for keys and values.
 */
void symbol_table_free(void) {
    hashmap_free(labels);
    hashmap_free(addresses);
}
