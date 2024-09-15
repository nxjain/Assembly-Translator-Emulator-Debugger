#ifndef DEBUGGING_H
#define DEBUGGING_H

#include <stdio.h>

//if DEBUGGING_MODE macro is defined, the macro gets substituted to a print function, otherwise gets removed
#ifdef DEBUGGING_MODE
#define debug_printf(...) do { \
    printf("DEBUG %s: line %d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); \
} while (0)
#else
#define debug_puts(message)
#define debug_printf(message, ...)
#endif

#endif
