/**
 * @file symbol_table.h
 * @brief Header file for symbol table management in assembly instructions.
 *
 * This header file declares functions for initializing a symbol table, adding labels with their
 * literal addresses, retrieving addresses of labels relative to instruction addresses,
 * and freeing allocated memory. It includes necessary external declarations for interfacing
 * with other modules such as DArray from "../ADTs/darray.h".
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>
#include "../ADTs/darray.h"

// Initialize the symbol table.
extern void symbol_table_init();

// Add a label with its corresponding address.
extern void symbol_table_add_label(DArray *instructions, uint32_t address, char *label);

// Get the offset from a label to an instruction.
extern int symbol_table_get_address(uint32_t address_of_instruction, char *label);

// Free the memory allocated for the symbol table.
extern void symbol_table_free();

#endif /* SYMBOL_TABLE_H */
