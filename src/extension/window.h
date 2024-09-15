#include <stdio.h>
#include <stdlib.h>
#include "../ADTs/darray.h"

extern void window_init(const char* input_file_path, DArray *assembly_lines, DArray *break_points_arr);
extern void window_refresh(void);
extern char *window_get_input(void);
extern void window_set_src_line(int line_number);
extern void window_print(const char *format, ...);
extern void window_free(void);