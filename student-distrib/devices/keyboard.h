#include "../i8259.h"
#include "../lib.h"
#include "terminal.h"

extern void initKeyboard();
extern void keyboardHandler();
extern void add_char_to_keyboard_buffer(char key);
char testKey;
unsigned int capsLock, shiftLock, ctrlLock, altLock;


#define IRQ_KEYBOARD    0x01
#define KEYBOARD_PORT   0x60

#define NUM_KEYS        63
#define KEYBOARD_BUFFER_SIZE 128

#define SHIFT           0xF
#define CTRL            0xE
#define CAPSLOCK        0x4
#define ENTER           0x1c
#define ALT             0x5       // change if later needed for other Checkpoints
#define L_SCAN_CODE     0x26
#define UNPRESS_SHIFTL  0xAA
#define UNPRESS_SHIFTR  0xB6
#define UNPRESS_ALT     0xB8

#define ENTER_SCAN      0x1C
#define BACKSPACE       0x0E
#define UNPRESS_CTRL    0x9D
#define F1              0x1
#define F2              0x2
#define F3              0x3


/*Keycodes source: https://wiki.osdev.org/Keyboard */
const static char keyCodes[NUM_KEYS] = {
// index offset| *******************************************************************|Backspace| Tap |
     0,    0,   '1',  '2',  '3',  '4',  '5',  '6', '7',  '8',  '9',  '0', '-',  '=' ,  '\b'   , '\t',
//  *********************************************************************|Enter|Ctrl LR|
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i', 'o',  'p',  '[',  ']', '\n' , CTRL ,
//  *********************************************************| ' |*****|Shift L|****
    'a',  's', 'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'', '`', SHIFT ,'\\',
//  ********************************************************|Shift R|   
    'z',  'x',  'c',  'v', 'b',  'n',  'm',  ',',  '.',  '/', SHIFT ,
// pad:*|AltLR|   |CapsLock |F1|F2|F3|
    '*' , ALT ,' ',CAPSLOCK ,F1,F2,F3 };

const static char shiftCodes[NUM_KEYS] = {
// index offset| ****************************************************************|Backspace| Tap |
     0,    0,   '!', '@', '#',  '$',  '%',  '^', '&', '*',  '(',  ')', '_',  '+' ,  '\b'   , '\t',
//  *********************************************************************|Enter|Ctrl LR|
    'Q',  'W', 'E', 'R',  'T',  'Y',  'U',  'I', 'O',  'P',  '{',  '}', '\n' , CTRL ,
//  *********************************************************| ' |****|Shift L|****
    'A',  'S', 'D', 'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"', '~', SHIFT ,'|',
//  ********************************************************|Shift R|   
    'Z',  'X',  'C',  'V', 'B',  'N',  'M',  '<',  '>',  '?', SHIFT ,
// pad:*|AltLR|   |CapsLock |F1|F2|F3|
    '*' , ALT ,' ',CAPSLOCK ,F1,F2,F3};

const static char capsCodes[NUM_KEYS] = {
// index offset| ****************************************************************|Backspace| Tap |
     0,    0,   '1',  '2',  '3',  '4',  '5',  '6', '7',  '8',  '9',  '0', '-',  '=' ,  '\b'   , '\t',
//  *********************************************************************|Enter|Ctrl LR|
    'Q',  'W', 'E', 'R',  'T',  'Y',  'U',  'I', 'O',  'P',  '[',  ']', '\n' , CTRL ,
//  *********************************************************| ' |****|Shift L|****
    'A',  'S', 'D', 'F',  'G',  'H',  'J',  'K',  'L',  ';', '\'', '`', SHIFT ,'\\',
//  ********************************************************|Shift R|   
    'Z',  'X',  'C',  'V', 'B',  'N',  'M',  ',',  '.',  '/', SHIFT ,
// pad:*|AltLR|   |CapsLock |F1|F2|F3|
    '*' , ALT ,' ',CAPSLOCK ,F1,F2,F3};

const static char shiftCapsCodes[NUM_KEYS] = {
// index offset| ****************************************************************|Backspace| Tap |
     0,    0,   '!', '@', '#',  '$',  '%',  '^', '&', '*',  '(',  ')', '_',  '+' ,  '\b'   , '\t',
//  *********************************************************************|Enter|Ctrl LR|
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i', 'o',  'p',  '{',  '}', '\n' , CTRL ,
//  *********************************************************| ' |****|Shift L|****
    'a',  's', 'd',  'f',  'g',  'h',  'j',  'k',  'l',  ':', '\"', '~', SHIFT ,'|',
//  ********************************************************|Shift R|   
    'z',  'x',  'c',  'v', 'b',  'n',  'm',  '<',  '>',  '?', SHIFT ,
// pad:*|AltLR|   |CapsLock |F1|F2|F3|
    '*' , ALT ,' ',CAPSLOCK ,F1,F2,F3};
