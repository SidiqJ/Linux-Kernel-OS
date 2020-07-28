#ifndef TERMINAL_H__
#define TERMINAL_H__

#include "../lib.h"
#include "../types.h"
//#include "../systemCalls/systemCalls.h"

#define KEYBOARD_BUFFER_SIZE 128
#define NUM_COLS    80
#define NUM_ROWS    25

int keyboard_buffer_idx, buffer_read_bytes;
char * passed_in_buffer;
char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
char terminal_buffer[KEYBOARD_BUFFER_SIZE];

int32_t dummy_fd; // for CP2 tests

int return_from_sys_write; // signal for shell (CP3)

int currentTerminal;



// Terminal Functions
extern int terminalOpen(const uint8_t* filename);
extern int terminalRead(int32_t fd, void* buf, int32_t nbytes);
extern int terminalWrite(int32_t fd, const void* buf, int32_t nbytes);
extern int terminalClose(int32_t fd);
extern void clear_buffer(int which_buffer);

// Cursor definitions
extern void setCursor (int x, int y);

#define CURSOR_PORT     0x3D4
#define CURSOR_DATA     0x3D5
#define LOCATION_HIGH   0xE
#define LOCATION_LOW    0xF
#define SCREEN_WIDTH    80

#define CLR_KEYBOARD_BUFFER 1
#define CLR_TERMINAL_BUFFER 2

#define MAX_TERMINAL_NUMBER 3

#define IRQ_KEYBOARD    0x01

// Struct for multiple terminals
typedef struct {
    char terminal_buffer[KEYBOARD_BUFFER_SIZE];
    char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
    int keyboard_buffer_idx;
    //cursor x,y
    int x, y;
    //pcb_t* top_process_ptr;
    int in_use;
    int terminal_num;

} terminal_info_t;

extern void switchTerminal(int terminalIndex);
extern void switchScreen(terminal_info_t * currentTerminal, terminal_info_t * newTerminal);
extern void initTerminals();




// Array of available terminals
terminal_info_t available_terminals[MAX_TERMINAL_NUMBER];
#endif /* Terminal_H__*/
