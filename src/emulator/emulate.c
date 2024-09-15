/**
 * @file emulate.c
 * @brief Source file for the "emulate" executeable file.
 * @details Reads in binary object code from a binary file, which is the first argument passed in, and runs the code.
 *          The emulator should also support an optional output file, supplied as the second argument.
 *          When no output file is specified, the emulator should print the results to stdout;
 *          when one is specified, the results should be saved in <file_out>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"

void emulate(const char *input_file_path, const char *output_file_path) {
  // Initialize CPU with instructions from input file
  init_cpu(input_file_path);
  // Run CPU simulation
  run_cpu();
  // Print CPU state to output file or stdout
  print_cpu(output_file_path);
}

/**
 * Main function for a simple CPU simulator.
 *
 * Parses command-line arguments for input and optionally output file paths.
 * Initializes the CPU with instructions from the input file.
 * Runs the CPU simulation.
 * Prints CPU state information to the specified output file or stdout.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return EXIT_SUCCESS if the program executes successfully, otherwise EXIT_FAILURE.
 */
int main(int argc, char **argv) {
  //parsing the arguments
  if (argc <= 1) {
    perror("Not enough arguments\n");
    return EXIT_FAILURE;
  }
  if (argc >= 4) {
    perror("Too many arguments\n");
    return EXIT_FAILURE;
  }
   
  const char *input_file_path;
  const char *output_file_path;

  if (argc == 2) {
    input_file_path  = argv[1];
    output_file_path = NULL;
  } else {
    input_file_path  = argv[1];
    output_file_path = argv[2];
  }

  emulate(input_file_path, output_file_path);

  return EXIT_SUCCESS;
}
