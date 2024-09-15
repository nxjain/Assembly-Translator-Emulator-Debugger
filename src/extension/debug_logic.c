#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "debug_logic.h"
#include "debug_info.h"
#include "window.h"
#include "../utils.h"
#include "../ADTs/darray.h"
#include "../ADTs/hashmap.h"
#include "../emulator/memory.h"
#include "../emulator/register.h"
#include "../emulator/cpu.h"
#include "../assembler/decode_helper.h"
#include "../assembler/decode.h"

#define INITIAL_BUFFER_SIZE 10
#define NO_LINE_HIGHLIGHT 0  // zero value removes the line highlight (indicating which line is running)

typedef enum{ARG_1, ARG_2, ARG_3, ARG_4, MAX_NUM_ARGUMENTS} ArgumentNumber;
typedef enum{PROGRAM_HALT = 0, PROGRAM_EXIT = 0, PROGRAM_CONTINUE} ProgramState;

static DArray *assembly_lines;
static DArray *breakpoints;
static HashMap *address_to_line;

static bool program_running = false;
static int cur_line_number = 1;

// -------------------------------- Standard Helper Functions ----------------------------------

/**
 * @brief Checks if a string is a number.
 * @param str The string to check.
 * @return true if the string is not a number, false otherwise.
 */
static bool string_is_number(const char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return true; // Non number found
        }
        str++;
    }
    return true; // No non numbers found.
}

/**
 * @brief Retrieves the line number from a string.
 * @param str The string containing the line number.
 * @return The integer line number, or 0 if invalid.
 */
static int get_line_number(const char *str){
    if (!string_is_number(str)){
        window_print("ERROR: Invalid number passed in.");
        return 0;
    }
    int line_num = atoi(str);
    if (line_num <= 0 || line_num > darray_length(assembly_lines)){
        window_print("ERROR: Line number out of range.");
        return 0;
    }
    return line_num;
}

/**
 * @brief Checks if user input matches a command reference.
 * @param user_input The user input to check.
 * @param cmd_ref The command reference to compare against.
 * @return true if the user input matches the command reference, false otherwise.
 */
static bool input_matches(const char *user_input, CommandRef cmd_ref){
    return !(strcmp(user_input, cmd_names[cmd_ref]) && strcmp(user_input, cmd_short_names[cmd_ref]));
}

// -------------------------------- Debugging Helper Functions ----------------------------------

/**
 * @brief Steps through one instruction in the debugger.
 * @return PROGRAM_HALT if a breakpoint or halt instruction is hit, PROGRAM_CONTINUE otherwise.
 */
bool debugger_step_instruction() {
    if(step_instruction()){
        cur_line_number = *(int*) hashmap_get(address_to_line, int_to_string(get_spec_register(PROGRAM_COUNTER)));
        window_set_src_line(cur_line_number);

        // Check if reached breakpoint
        if (darray_index_of(breakpoints, &cur_line_number, int_cmp) != -1){
            window_print("-----Breakpoint reached: Line %d-----", cur_line_number);
            return PROGRAM_HALT;
        }
        return PROGRAM_CONTINUE;
    }
    window_print("***End of program reached***");
    program_running = false;
    window_set_src_line(NO_LINE_HIGHLIGHT);
    return PROGRAM_HALT;
}

/**
 * @brief Runs step instruction debug until a halt instruction or breakpoint is hit.
 */
void debugger_run_cpu() {
    // Run step instruction - if false returned then halt/breakpoint instruction received.
    while(debugger_step_instruction());
}

/**
 * @brief Handles invalid user input for a command.
 * @param user_input The invalid user input.
 * @param cmd_ref The command reference that was attempted.
 * @return PROGRAM_CONTINUE to allow further execution.
 */
static int invalid_user_input(char *user_input, CommandRef cmd_ref){
    // Invalid argument inputted:
    window_print("Illegal arguments passed in %s: %s", cmd_ref != CMD_NULL ? cmd_names[cmd_ref] : "", user_input);
    return PROGRAM_CONTINUE;
}

/**
 * @brief Loads assembly code from a file into memory for debugging.
 * @param input_file_path Path to the input file.
 */
static void debugger_load_assembly(const char *input_file_path) {
    FILE *input_file = fopen(input_file_path, "r");
    if (input_file == NULL) {
        fprintf(stderr, "Failed to open file %s\n", input_file_path);
        exit(EXIT_FAILURE);
    }

    assert(input_file != NULL);
    
    int buffer_size = INITIAL_BUFFER_SIZE;
    char *buffer    = malloc(buffer_size * sizeof(char));
    int length      = 0;
    
    char c;

    while ((c = fgetc(input_file)) != EOF) {
        if (c == '\n') {
            buffer[length] = '\0';
            darray_add(assembly_lines, buffer);

            buffer_size = INITIAL_BUFFER_SIZE;
            buffer      = malloc(buffer_size * sizeof(char));
            length      = 0;
            continue;
        } 

        buffer[length++] = c;

        if (length == buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            assert(buffer != NULL);
        }
    }

    if (length != 0) { //last line did not end with \n
        buffer[length] = '\0';
        darray_add(assembly_lines, buffer);
    } else {
        free(buffer);
    }

    fclose(input_file);
}

/**
 * @brief Resets CPU and memory to initial state for debugging.
 */
static void debugger_reset_memory(){
    init_register();
    init_memory();
    load_instructions_to_memory_array(decode_get_instructions());
    cur_line_number = 1;
}

// -------------------------------- Debugging Printing Functions ----------------------------------

/**
 * @brief Prints non-zero memory contents.
 */
static void debugger_print_memory(){
    window_print("Non-Zero Memory:\n");
    for (int i = 0; i < NUM_OF_MEMORY_ADDRESS; i += 4) {
        if (get_word(i) != 0) {
            window_print("0x%08x: %08x\n", i, get_word(i));
        }
    }
}

/**
 * @brief Prints all registers and their values.
 */
static void debugger_print_registers(){
    window_print("Registers:\n");
    for (int i = 0; i < NUM_REGISTERS - 1; i = i + 5) {
        window_print("X%02d = %016lx   X%02d = %016lx   X%02d = %016lx   X%02d = %016lx   X%02d = %016lx\n", 
        i, get_reg_value_64(i), i+1, get_reg_value_64(i+1), i+2, get_reg_value_64(i+2), i+3, get_reg_value_64(i+3),
        i+4, get_reg_value_64(i+4));
    }
    window_print("X%02d = %016lx   X%02d = %016lx   PC  = %016lx", NUM_REGISTERS-1, get_reg_value_64(NUM_REGISTERS-1),
     NUM_REGISTERS, get_reg_value_64(NUM_REGISTERS), get_spec_register(PROGRAM_COUNTER));
}

/**
 * @brief Prints processor state flags.
 */
static void debugger_print_pstates(){
    processor_state pstate = get_pstate();
    window_print("PSTATE : %s%s%s%s\n", \
    pstate.negative_flag ? "N" : "-", \
    pstate.zero_flag     ? "Z" : "-", \
    pstate.carry_flag    ? "C" : "-", \
    pstate.overflow_flag ? "V" : "-");
}

/**
 * @brief Prints all set breakpoints.
 */
static void debugger_print_breakpoints(){
    if (darray_length(breakpoints) == 0){
        window_print("Breakpoints is empty");
    }
    window_print("Breakpoints:");
    for (int i = 0; i < darray_length(breakpoints); i++) {
        window_print("Breakpoint at line %d", *(int *)darray_get(breakpoints, i));
    }
}

/**
 * @brief Prints help information for available commands.
 */
static void debugger_print_help(){
    window_print("List of ARMv8 commands:");
    for (CommandRef cur_cmd = 0; cur_cmd < NUM_HELP_COMMANDS; cur_cmd++){
        window_print("%-10s: %s", cmd_names[cur_cmd], cmd_help[cur_cmd]);
    }
}

/**
 * @brief Prints detailed help information for a specific command.
 * @param user_input The command for which help is requested.
 */
static void debugger_print_help_cmd(char *user_input){
    for (CommandRef cur_cmd = 0; cur_cmd < NUM_HELP_COMMANDS; cur_cmd++){
        if (input_matches(user_input, cur_cmd)){
            window_print(" - %s", cmd_help[cur_cmd]);
            window_print(" - %s", cmd_syntax[cur_cmd]);
            if (cur_cmd >= CMD_BREAKPOINT && cur_cmd < NUM_HELP_COMMANDS){
                window_print(" - %s", cmd_examples[cur_cmd]);
            }
            return;
        }
    }
    invalid_user_input(user_input, CMD_HELP);
}

// -------------------------------- Debugging Main Functions ----------------------------------

/**
 * @brief Parses user input and executes corresponding debugger commands.
 * @param user_input The input string from the user.
 * @return PROGRAM_EXIT if the user wishes to exit using 'q', PROGRAM_CONTINUE otherwise.
 */
static bool parse_input(char *user_input){
    // Initialise arguments array
    char *arguments[MAX_NUM_ARGUMENTS];
    for (int i = 0; i < MAX_NUM_ARGUMENTS; i++) {
        arguments[i] = NULL;
    }

    int num_args = 0;
    // Get the first argument
    char *segment = strtok(user_input, " ");

    // Walk through other arguments
    while (segment != NULL && num_args < MAX_NUM_ARGUMENTS) {
        arguments[num_args] = segment;
        num_args++;
        segment = strtok(NULL, " ");
    }

    if (num_args == 1){
        if (input_matches(arguments[ARG_1], CMD_RUN)){
            // Check if program already running:
            if (program_running){
                // Ask user if they are sure they want to restart:
                window_print("The program is currently running, are you sure you want to start again? (y/n): ");
                char *user_response = window_get_input();
                while (strcmp(user_response, "y") != 0 && strcmp(user_response, "n") != 0){
                    window_print("Please enter 'y' or 'n'.");
                    user_response = window_get_input();
                }
                // Guaranteed that user response is just one character - 'y' or 'n':
                if (user_response[FST_CHAR_INDEX] == 'n'){
                    window_print("Resuming program: ");
                    return PROGRAM_CONTINUE;
                }
                window_print("Restarting program: ");
            }
            program_running = true;
            debugger_reset_memory();
            debugger_run_cpu();
            return PROGRAM_CONTINUE;
        }
        if (input_matches(arguments[ARG_1], CMD_QUIT)){
            return PROGRAM_EXIT;
        }
        if (input_matches(arguments[ARG_1], CMD_CONTINUE)){
            if (!program_running){
                window_print("The program has not started yet.");
                return PROGRAM_CONTINUE;
            }
            debugger_run_cpu();
            return PROGRAM_CONTINUE;
        }
        if (input_matches(arguments[ARG_1], CMD_NEXT)){
            if (!program_running){
                window_print("The program has not started yet.");
                return PROGRAM_CONTINUE;
            }
            debugger_step_instruction();
            return PROGRAM_CONTINUE;
        }
        if (input_matches(arguments[ARG_1], CMD_REFRESH)){
            window_refresh();
            return PROGRAM_CONTINUE;
        }
        if (input_matches(arguments[ARG_1], CMD_HELP)){
            debugger_print_help();
            return PROGRAM_CONTINUE;
        }

        return invalid_user_input(user_input, CMD_NULL);
    }

    if (num_args == 2) {
        if (input_matches(arguments[ARG_1], CMD_BREAKPOINT)){
            int line_num = get_line_number(arguments[ARG_2]);
            // Check for invalid line number - 0 is returned if line_number was invalid.
            if (!line_num){
                return PROGRAM_CONTINUE;
            }

            // Add breakpoint to breakpoints array:
            int32_t *line_num_ptr = malloc_assert_num(line_num);
            darray_add(breakpoints, line_num_ptr);
            // Refresh so the breakpoint indicator shows up:
            window_refresh();
            return PROGRAM_CONTINUE;
        }

        if (input_matches(arguments[ARG_1], CMD_CLEAR)){
            int line_num = get_line_number(arguments[ARG_2]);
            // Check for invalid line number:
            if (!line_num){
                return PROGRAM_CONTINUE;
            }
            int line_num_index = darray_index_of(breakpoints, &line_num, int_cmp);
            if (line_num_index == -1){
                window_print("Breakpoint does not exist");
                return PROGRAM_CONTINUE;
            }
            darray_remove(breakpoints, line_num_index);
            window_refresh();
            return PROGRAM_CONTINUE;
        }

        if (input_matches(arguments[ARG_1], CMD_PRINT)){
            // If Register Access:
            if (is_valid_register(arguments[ARG_2])){
                uint64_t reg_value;
                int reg_num = read_reg_value(arguments[ARG_2]);
                uint32_t reg_index = read_reg_value(arguments[ARG_2]);
                if (reg_index < 0 || reg_index > NUM_REGISTERS){
                    window_print("Register value provided out of range.");
                    return PROGRAM_CONTINUE;
                }

                if (is_bit_mode_64(arguments[ARG_2])){
                    reg_value = get_reg_value_64(reg_num);
                } else if (is_bit_mode_32(arguments[ARG_2])){
                    reg_value = get_reg_value_32(reg_num);
                } else{ //Zero register
                    reg_value = 0;
                }
                window_print("X%02d = 0x%08lx", reg_index, reg_value);

                return PROGRAM_CONTINUE;
            }
            // If Memory Access:
            if (arguments[ARG_2][FST_CHAR_INDEX] == '*' && is_hex_number(arguments[ARG_2] + 1)){
                uint32_t mem_value = get_word(read_imm_value(arguments[ARG_2]));
                window_print("*%02d = 0x%08x", read_imm_value(arguments[ARG_2]), mem_value);
                return PROGRAM_CONTINUE;
            }

            // Invalid memory or register inputted:
            return invalid_user_input(user_input, CMD_PRINT);
        }

        if (input_matches(arguments[ARG_1], CMD_INFO)){
            if (input_matches(arguments[ARG_2], CMD_MEMORY)){
                debugger_print_memory();
                return PROGRAM_CONTINUE;
            }

            if (input_matches(arguments[ARG_2], CMD_REGISTERS)){
                debugger_print_registers();
                return PROGRAM_CONTINUE;
            }

            if (input_matches(arguments[ARG_2], CMD_PSTATE)){
                debugger_print_pstates();
                return PROGRAM_CONTINUE;
            }

            if (input_matches(arguments[ARG_2], CMD_BREAKPOINTS)){
                debugger_print_breakpoints();
                return PROGRAM_CONTINUE;
            }
            // Invalid argument inputted:
            return invalid_user_input(user_input, CMD_INFO);
        }

        if (input_matches(arguments[ARG_1], CMD_HELP)){
            debugger_print_help_cmd(arguments[ARG_2]);
            return PROGRAM_CONTINUE;
        }

        return invalid_user_input(user_input, CMD_NULL);
    }

    if (num_args == 4){
         if (input_matches(arguments[ARG_1], CMD_SET) && strcmp(arguments[ARG_3], "=") == 0 && string_is_number(arguments[ARG_4])){
            //Registers:
            if (is_valid_register(arguments[ARG_2])){
                // Cannot write to zero register:
                if (is_zero_register(arguments[ARG_2])){
                    window_print("Cannot write to zero register.");
                    return PROGRAM_CONTINUE; 
                }

                uint64_t new_value = atoi(arguments[ARG_4]);
                uint32_t reg_index = read_reg_value(arguments[ARG_2]);
                if (reg_index < 0 || reg_index > NUM_REGISTERS){
                    window_print("Register value provided out of range.");
                    return PROGRAM_CONTINUE;
                }
                set_reg_value(reg_index, new_value);
                window_print("X%02d := 0x%08lx", reg_index, new_value);
                return PROGRAM_CONTINUE;
            }
            //Memory:

            if (arguments[ARG_2][FST_CHAR_INDEX] == '*' && is_hex_number(arguments[ARG_2]+1)){
                set_word(read_imm_value(arguments[ARG_2]), atoi(arguments[ARG_4]));
                window_print("*%02d := 0x%08x", read_imm_value(arguments[ARG_2]), atoi(arguments[ARG_4]));
                return PROGRAM_CONTINUE;
            }

            return invalid_user_input(user_input, CMD_SET);
        }
        // Invalid instruction inputted:
        return invalid_user_input(user_input, CMD_NULL);
    }
    
    // Invalid instruction inputted:
    return invalid_user_input(user_input, CMD_NULL);
}

/**
 * @brief Initializes the debugger with assembly code and breakpoints.
 * @param input_file_path Path to the input assembly file.
 */
void debugger_init(const char *input_file_path) {
    assembly_lines = darray_init(free);
    address_to_line = hashmap_init(free);
    breakpoints = darray_init(free);
    decode_init();
    
    debugger_load_assembly(input_file_path);
    for (int line_num = 1; line_num <= darray_length(assembly_lines); line_num++) {
        decode_debug(darray_get(assembly_lines, line_num-1), address_to_line, line_num);
    }

    load_instructions_to_memory_array(decode_get_instructions());

    window_init(input_file_path, assembly_lines, breakpoints);
}

/**
 * @brief Runs the debugger loop, handling user input until exit.
 */
void debugger_run(void) {
    window_set_src_line(NO_LINE_HIGHLIGHT);
    // Function only returns false when "q" is entered.
    while (parse_input(window_get_input()));
}

/**
 * @brief Frees allocated memory and resources used by the debugger.
 */
void debugger_free(void) {
    darray_free(assembly_lines);
    hashmap_free(address_to_line);
    darray_free(breakpoints);
    window_free();
}
