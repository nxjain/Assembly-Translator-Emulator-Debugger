#include <stdlib.h>
#include <stdio.h>

#include "debug_logic.h"

void debugger(const char *input_file_path) {
    debugger_init(input_file_path);
    debugger_run();
    debugger_free();
}

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./emulate_debug input.s");
        exit(EXIT_FAILURE);
    }

    const char *input_file_path = argv[1];

    debugger(input_file_path);
    return EXIT_SUCCESS;
}
