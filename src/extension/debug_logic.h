/**
 * @file debugger.h
 * @brief Header file for the ARMv8 debugger.
 * 
 * Contains declarations for functions related to debugger initialization,
 * running the debugger, and freeing resources.
 */

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdbool.h>

/**
 * @brief Initializes the debugger with the specified input file.
 * @param input_file_path The path to the input assembly file.
 */
extern void debugger_init(const char *input_file_path);

/**
 * @brief Runs the debugger, allowing user interaction.
 * This function handles user input and executes debugger commands
 * until the user chooses to exit.
 */
extern void debugger_run(void);

/**
 * @brief Frees resources allocated by the debugger.
 * Cleans up memory used by the debugger components and prepares
 * for program termination.
 */
extern void debugger_free(void);

#endif /* DEBUGGER_H */
