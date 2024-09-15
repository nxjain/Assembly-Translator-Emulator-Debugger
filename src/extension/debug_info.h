/**
 * @file debug_info.h
 * @brief Definitions of commands and their associated strings for an ARMv8 debugger.
 * 
 * This file defines enums and constant arrays for commands, their shortened versions,
 * help descriptions, syntax examples, and more. These are used throughout the debugger
 * interface to handle user commands effectively.
 */

// Define the enum to reference each string - NUM_HELP_COMMANDS used later in print_help
typedef enum {
    CMD_RUN, CMD_QUIT, CMD_CONTINUE, CMD_NEXT, CMD_REFRESH, CMD_BREAKPOINT, CMD_CLEAR,
    CMD_PRINT, CMD_SET, CMD_INFO, CMD_HELP, NUM_HELP_COMMANDS,
    CMD_MEMORY, CMD_REGISTERS, CMD_PSTATE, CMD_BREAKPOINTS, CMD_NULL,
} CommandRef;

// Define a const list of command name strings
const char* const cmd_names[] = {
    [CMD_RUN] = "run",
    [CMD_QUIT] = "quit",
    [CMD_CONTINUE] = "continue",
    [CMD_NEXT] = "next",
    [CMD_REFRESH] = "refresh",
    [CMD_BREAKPOINT] = "break",
    [CMD_CLEAR] = "clear",
    [CMD_PRINT] = "print",
    [CMD_SET] = "set",
    [CMD_HELP] = "help",
    [CMD_INFO] = "info",
    [CMD_MEMORY] = "memory",
    [CMD_REGISTERS] = "registers",
    [CMD_PSTATE] = "pstate",
    [CMD_BREAKPOINTS] = "breakpoints",
};

// Define a const list of shortened command name strings
const char* const cmd_short_names[] = {
    [CMD_RUN] = "r",
    [CMD_QUIT] = "q",
    [CMD_CONTINUE] = "c",
    [CMD_NEXT] = "n",
    [CMD_REFRESH] = "ref",
    [CMD_BREAKPOINT] = "b",
    [CMD_CLEAR] = "cl",
    [CMD_PRINT] = "p",
    [CMD_SET] = "s",
    [CMD_HELP] = "h",
    [CMD_INFO] = "i",
    [CMD_MEMORY] = "mem",
    [CMD_REGISTERS] = "reg",
    [CMD_PSTATE] = "pst",
    [CMD_BREAKPOINTS] = "brs",
};

// Define a const list of help descriptions for each command
const char* const cmd_help[] = {
    [CMD_RUN] = "Start/Restart program execution",
    [CMD_QUIT] = "Exit ARMv8 Debugger",
    [CMD_CONTINUE] = "Continue program execution",
    [CMD_NEXT] = "Step program",
    [CMD_REFRESH] = "Refresh screen display",
    [CMD_BREAKPOINT] = "Set a breakpoint at specified line number",
    [CMD_CLEAR] = "Delete a breakpoint at a specified line number",
    [CMD_PRINT] = "Print value of register or memory",
    [CMD_SET] = "Assign value to a general register or a memory location",
    [CMD_INFO] = "Show information about all registers, non-zero memory locations or the program state",
    [CMD_HELP] = "Show information about a specified command, or all commands",
};

// Define a const list of syntax examples for each command
const char* const cmd_syntax[] = {
    [CMD_RUN] = "Type 'r' or \"run\".",
    [CMD_QUIT] = "Type 'q' or \"quit\".",
    [CMD_CONTINUE] = "Type 'c' or \"continue\".",
    [CMD_NEXT] = "Type 'n' or \"next\".",
    [CMD_REFRESH] = "Type \"ref\" or \"refresh\".",
    [CMD_BREAKPOINT] = "Type 'b' or \"break\".",
    [CMD_CLEAR] = "Type \"cl\" or \"clear\".",
    [CMD_PRINT] = "Type 'p' or \"print\"",
    [CMD_SET] = "Type 's' or \"set\"",
    [CMD_INFO] = "Type 'i' or \"info\"",
    [CMD_HELP] = "Type 'h' or \"help\"",
};

// Define a const list of examples for each command
const char* const cmd_examples[] = {
    [CMD_BREAKPOINT] = "Example: b 5 - Creates a breakpoint on line 5.",
    [CMD_CLEAR] = "Example: c 5 - Removes a breakpoint on line 5 if it exists.",
    [CMD_PRINT] = "Example: p x30/*0x4 - Prints the value held at register x30/memory address 0x4",
    [CMD_SET] = "Example: s x0/*0x4 = 5 - Sets the value held at register x0/memory address 0x4 equal to 5",
    [CMD_INFO] = "Example: i bs - Prints the location of all breakpoints",
    [CMD_HELP] = "Example: h run - Prints information about the command \"run\"",
};
