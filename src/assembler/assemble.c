#include <stdlib.h>
#include <assert.h>

//turns on debug mode
#define DEBUGGING_MODE

#include "../utils.h"
#include "../debugging.h"
#include "../ADTs/darray.h"
#include "decode.h"
#include "decode_helper.h"

#define INITIAL_BUFFER_SIZE 10

/**
 * @brief Reads each line and calls the call back function. Ignores empty lines
 *
 * @param output_file_path Path to the input assembly file.
 * @param call_back Call back function to run on every line.
 */
static void for_each_line_in_file(const char* input_file_path, void (*call_back)(char *line)) {
  FILE *input_file = fopen(input_file_path, "r");
  if (input_file == NULL) {
    fprintf(stderr, "Failed to open file %s\n", input_file_path);
    exit(EXIT_FAILURE);
  }

  assert(input_file != NULL);
  assert(call_back != NULL);

  int buffer_size = INITIAL_BUFFER_SIZE;
  char *buffer    = malloc(buffer_size * sizeof(char));
  int length      = 0;
  
  char c;

  while ((c = fgetc(input_file)) != EOF) {
    if (c == '\n') {
      if (length == 0) continue; //empty line

      buffer[length] = '\0';
      length = 0;
      call_back(buffer);
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
    call_back(buffer);
  }

  free(buffer);
}

/**
 * @brief Writes an array of instructions to a binary file.
 *
 * This function opens a binary file specified by `output_file_path` in write mode ("wb").
 * It then iterates through the provided dynamic array `instructions`, writing each uint32_t
 * element to the file. If any errors occur during file operations, such as failure to open
 * or write to the file, the function prints an error message to stderr and exits the program.
 *
 * @param output_file_path Path to the output binary file.
 * @param instructions Dynamic array (DArray) containing uint32_t instructions to write.
 */
static void write_to_binray_file(const char *output_file_path, DArray *instructions) {
  FILE *output_file = fopen(output_file_path, "wb");
  if (output_file == NULL) {
    fprintf(stderr, "Failed to open file %s\n", output_file_path);
    exit(EXIT_FAILURE);
  }

  assert_msg(instructions != NULL, "Instructions DArray passed in is empty\n");

  int index = 0;
  uint32_t *pbinary;

  while (darray_iterator(instructions, &index, (void **) &pbinary)) {
    assert_msg(fwrite(pbinary, sizeof(uint32_t), 1, output_file) == 1, "Failed to write to file %s\n", output_file_path);
  }

  fclose(output_file);
}

void assemble (const char *input_file_path, const char *output_file_path) {
  // Initialize decoding process
    decode_init();

    // Decode each line of the input file
    for_each_line_in_file(input_file_path, decode);

    // Get decoded instructions
    DArray *instructions = decode_get_instructions();
    
    // Write instructions to binary output file
    write_to_binray_file(output_file_path, instructions);

    // Free resources used during decoding
    decode_free();
}

/**
 * Main function for an assembly language assembler.
 *
 * Parses command-line arguments for input and output file paths.
 * Initializes the decoding process.
 * Decodes each line of the input file into instructions.
 * Writes the decoded instructions to the output binary file.
 * Frees resources used during decoding.
 *
 * @param argc Number of command-line arguments (must be 3).
 * @param argv Array of command-line argument strings containing input-file and output-file paths.
 * @return EXIT_SUCCESS if the program executes successfully, otherwise EXIT_FAILURE.
 */
int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: ./assemble input-file output-file\n");
    return EXIT_FAILURE;
  }

  char *input_file_path = argv[1];
  char *output_file_path = argv[2];

  assemble(input_file_path, output_file_path);

  return EXIT_SUCCESS;
}
