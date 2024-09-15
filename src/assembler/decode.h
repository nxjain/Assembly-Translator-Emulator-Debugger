/**
 * @file decode.h
 * @brief Header file for decoding assembly instructions into 32-bit machine code.
 *
 * This header file declares functions for initializing decoding, decoding assembly instructions,
 * performing temporary testing, retrieving decoded instructions, and freeing allocated memory.
 * It includes necessary external declarations for interfacing with other modules such as symbol_table.h.
 */

#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>
#include "symbol_table.h"
#include "../ADTs/hashmap.h"

extern void decode_init(void);
extern void decode(char * assembly_line);
extern DArray *decode_get_instructions(void);
extern void decode_free(void);

extern void decode_debug(char *assembly_line_input, HashMap* address_to_line, uint32_t line_num);
#endif
