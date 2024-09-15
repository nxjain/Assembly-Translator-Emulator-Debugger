#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>

#include "window.h"
#include "../ADTs/darray.h"

#define INITIAL_BUFFER_SIZE 10
#define HEADER "(debug) "
#define HEADER_PADDING 10

#define LINE_PADDING 2
#define TITLE_LINE 0
#define SRC_START_LINE 1
#define SRC_END_LINE (window_y - 1)

#define CMD_START_LINE 1
#define CMD_END_LINE (window_y - 2)

#define ESCAPE_CHAR '\033'
#define FOCUSE "[I"
#define UNFOCUSE "[O"

#define SRC_BOX_COLOR 1

typedef struct {
    WINDOW *window;
    DArray *lines;
    int start_line;
} window;

static const char *assembly_file_name;

static window src;
static window cmd;

//Current window size of each window
static int window_y;
static int window_x;

static int buffer_size;
static int length;
static char *buffer;

static DArray *break_points;
static int current_instruction_line; // keeps track of which line to highlight

static bool terminal_focused;

static char *previous_command;

static int compare_int(const void *element1, const void *element2) {
    const int *num1 = element1;
    const int *num2 = element2;
    return *num1 - *num2;
}

static void display_source() {
    // Clear existing content in window
    werase(src.window);

    // Box around windows for visual separation
    wattron(src.window, COLOR_PAIR(SRC_BOX_COLOR));
    box(src.window, 0, 0);

    // Print information in source window
    mvwprintw(src.window, TITLE_LINE, LINE_PADDING, "%s", assembly_file_name);
    wattroff(src.window, COLOR_PAIR(SRC_BOX_COLOR));

    // Read lines from file and display in source window
    for (int window_line = SRC_START_LINE, src_line = src.start_line; window_line < SRC_END_LINE && src_line <= darray_length(src.lines); window_line++, src_line++) {
        char *line = darray_get(src.lines, src_line - 1);
        char *break_point = darray_index_of(break_points, &src_line, compare_int) == -1 ? "  " : "b+";

        if (src_line == current_instruction_line) {
            wattron(src.window, A_REVERSE);
            mvwprintw(src.window, window_line, LINE_PADDING, "%s%4d  %s", break_point, src_line, line);
            wattroff(src.window, A_REVERSE);
        } else {
            mvwprintw(src.window, window_line, LINE_PADDING, "%s%4d  %s", break_point, src_line, line);
        }
    }

    wrefresh(src.window);
}

static void display_command() {
    // Clear existing content in window
    werase(cmd.window);

    // Box around windows for visual separation
    box(cmd.window, 0, 0);

    // Print information in command window
    mvwprintw(cmd.window, TITLE_LINE, LINE_PADDING, "Command Line Interface:");
    if (darray_length(cmd.lines) == 0) {
        mvwprintw(cmd.window, CMD_START_LINE, LINE_PADDING, "Type commands here, press 'q' to quit");
    }

    // Print commands starting from line 1
    for (int window_line = CMD_START_LINE, cmd_line = cmd.start_line; window_line < CMD_END_LINE && cmd_line <= darray_length(cmd.lines); window_line++, cmd_line++) {
        char *line = darray_get(cmd.lines, cmd_line - 1);
        mvwprintw(cmd.window, window_line, LINE_PADDING, "%s", line);
    }

    // Print buffer in COMMAND_END_LINE
    mvwprintw(cmd.window, CMD_END_LINE, LINE_PADDING, HEADER);
    for (int i = 0; i < length; i++) {
        mvwaddch(cmd.window, CMD_END_LINE, HEADER_PADDING + i, buffer[i]);
    }
    int cursor = terminal_focused ? (' ' | A_REVERSE) : ' ';

    mvwaddch(cmd.window, CMD_END_LINE, HEADER_PADDING + length, cursor);


    wrefresh(cmd.window);
}

void window_init(const char *input_file_path, DArray *assembly_lines, DArray *break_points_arr) {
    assembly_file_name = input_file_path;

    src.lines      = assembly_lines;
    src.start_line = SRC_START_LINE; // start from line number 1
    cmd.lines      = darray_init(free);
    cmd.start_line = CMD_START_LINE; // start from line number 1

    // Initialize buffer
    buffer_size = INITIAL_BUFFER_SIZE;
    length = 0;
    buffer = malloc(buffer_size * sizeof(char));
    assert(buffer != NULL);

    // Initialize break points
    break_points = break_points_arr;
    current_instruction_line = 0;

    previous_command = NULL;

    // Initialize curses
    initscr();
    noecho();
    curs_set(0); // Hide cursor
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL); // Enable mouse events

    terminal_focused = true; // Enable focus events
    printf("\033[?1004h"); 
    fflush(stdout);

    start_color();
    use_default_colors();
    init_pair(SRC_BOX_COLOR, COLOR_BLACK, COLOR_CYAN); // Initialize color

    // Get screen dimensions
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    window_y = max_y / 2;
    window_x = max_x;

    // Create windows for source and command line
    src.window = newwin(window_y, window_x, 0, 0);
    cmd.window = newwin(window_y, window_x, window_y, 0);

    keypad(src.window, TRUE);
    keypad(cmd.window, TRUE);

    display_source();
    display_command();
}

void window_refresh(void) {
     // Get new screen dimensions
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    window_y = max_y / 2;
    window_x = max_x;

    // Resize and reposition source window
    wresize(src.window, window_y, window_x);
    mvwin(src.window, 0, 0);

    // Resize and reposition command window
    wresize(cmd.window, window_y, window_x);
    mvwin(cmd.window, window_y, 0);

    werase(stdscr);

    // Redisplay content in source window
    display_source();
    display_command();
}

char *window_get_input(void) {
    int c;
    MEVENT event;

    char next[2];

    while ((c = wgetch(cmd.window)) != '\n') {
        switch (c) {
            case KEY_RESIZE:
                window_refresh();
                continue;
            case KEY_UP:
                if (src.start_line > SRC_START_LINE) {
                    src.start_line--;
                    display_source();
                }
                continue;
            case KEY_DOWN:
                if (src.start_line + SRC_END_LINE - 1 <= darray_length(src.lines)) { 
                    src.start_line++;
                    display_source();
                }
                continue;
            case KEY_MOUSE:
                if (getmouse(&event) == OK && wenclose(src.window, event.y, event.x)) {
                    if (event.bstate & BUTTON4_PRESSED && src.start_line > SRC_START_LINE) { //scroll up
                        src.start_line--;
                        display_source();
                    } else if (
                        event.bstate & BUTTON5_PRESSED && 
                        src.start_line + SRC_END_LINE - 1 <= darray_length(src.lines)
                    ) { //scroll down
                        src.start_line++;
                        display_source();
                    }
                }
                continue;
            case KEY_BACKSPACE:
                if (length > 0) {
                    length--;
                    display_command();
                }
                continue;
            case ESCAPE_CHAR:
                next[0] = wgetch(cmd.window);
                next[1] = wgetch(cmd.window);

                if (strncmp(next, FOCUSE, 2) == 0) { // when the terminal gains focus
                    terminal_focused = true;
                    display_command();
                    continue;
                } else if (strncmp(next, UNFOCUSE, 2) == 0) {  // when the terminal loses focus
                    terminal_focused = false;
                    display_command();
                    continue;
                }
                continue;
            default:
                if (isprint(c)) {
                    buffer[length++] = c;
                    display_command();

                    if (length == buffer_size) { //resize buffer
                        buffer_size *= 2;
                        buffer = realloc(buffer, buffer_size * sizeof(char));
                        assert(buffer != NULL);
                    }
                }
                continue;
        }
    }

    //null terminate the string
    buffer[length] = '\0';
    darray_add(cmd.lines, buffer);

    if (darray_length(cmd.lines) >= CMD_END_LINE) {
        cmd.start_line++;
    }
    
    char *input;

    if (length == 0 && previous_command != NULL) { // return the previous command if empty line is inputted
        input = strdup(previous_command);
        assert(input != NULL);
    } else {
        input = strdup(buffer); 
        assert(input != NULL);
        previous_command = buffer;
    }

    //reset buffer
    buffer_size = INITIAL_BUFFER_SIZE;
    length = 0;
    buffer = malloc(buffer_size * sizeof(char));
    assert(buffer != NULL);

    //refresh display
    display_command();

    return input;
}

void window_set_src_line(int line_number) {
    int max_lines = darray_length(src.lines);

    // if it is out of bounds, remove the highlight
    if (line_number <= 0 || max_lines < line_number) {
        current_instruction_line = line_number;
        display_source();
        return;
    }

    if (max_lines <= SRC_END_LINE || line_number <= (window_y / 2)) {
        src.start_line = 1;
    } else if (max_lines - line_number <= (window_y / 2)) {
        src.start_line = max_lines - window_y + 3;
    } else {
        src.start_line = line_number - (window_y / 2) + 1;
    }

    current_instruction_line = line_number;

    display_source();
}

void window_print(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // Determine the length of the formatted message
    int length = vsnprintf(NULL, 0, format, args);
    assert(length >= 0);
    va_end(args);

    // Allocate buffer for the formatted message
    char *message = (char *)malloc(length + 1);
    assert(message != NULL);

    // Print formatted message to buffer
    va_start(args, format);
    vsnprintf(message, length + 1, format, args);
    va_end(args);

    darray_add(cmd.lines, message);

    if (darray_length(cmd.lines) >= CMD_END_LINE) {
        cmd.start_line++;
    }

    display_command();
}

void window_free(void) {
    free(buffer);
    free(cmd.lines);
    delwin(src.window);
    delwin(cmd.window);
    endwin();
}

