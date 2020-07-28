#include "keyboard.h"

/* initKeyboard
 *   INPUTS: none
 *   RETURN VALUE: none     
 *   Function: Enable keyboard interrupt line */
void initKeyboard(){
    enable_irq(IRQ_KEYBOARD);
    clear();
    capsLock =0 , shiftLock = 0, ctrlLock =0, altLock = 0;
    keyboard_buffer_idx = 0;
    return_from_sys_write = 0;
}

/* keyboardHandler
 *   DESCRIPTION: Handle Keyboard interrupts
 *   INPUTS: none
 *   RETURN VALUE: none
 *   Function: print pressed key */
void keyboardHandler(){
    unsigned long flags;
    cli_and_save(flags); // disable Interrupts and save flags before handling Interrupt

    uint32_t scancode = inb(KEYBOARD_PORT);
    while (!scancode) { scancode = inb(KEYBOARD_PORT); } // get pressed key from KEYBOARD PORT

    switch (scancode){
    // Release shift button
    case UNPRESS_SHIFTL: shiftLock = 0; break;
    case UNPRESS_SHIFTR: shiftLock = 0; break;
    // Release CTRL button
    case UNPRESS_CTRL  : ctrlLock  = 0; break;
    // Release ALT Buttom
    case UNPRESS_ALT   : altLock   = 0; break;

    }

    
    // printf("%d", scancode); // only there for testing
    if (scancode <NUM_KEYS && keyCodes[scancode] && keyboard_buffer_idx < KEYBOARD_BUFFER_SIZE) //print char only when key is PRESSED && VALID
    {  
        switch (keyCodes[scancode]){

            case '\t': printf("    "); 
                       add_char_to_keyboard_buffer(' '); add_char_to_keyboard_buffer(' ');
                       add_char_to_keyboard_buffer(' '); add_char_to_keyboard_buffer(' ');
                       break;// tab print 4 spaces
                    
            case CAPSLOCK: capsLock= ~capsLock; break;
            case SHIFT   : shiftLock =1; break;
            case CTRL    : ctrlLock =1; break;
            case ALT     : altLock = 1; break;
            
            // case F1      : if(altLock){switchTerminal(0);} break;
            // case F2      : if(altLock){switchTerminal(1);} break;
            // case F3      : if(altLock){switchTerminal(2);} break;

            case F1      : switchTerminal(0); break;
            case F2      : switchTerminal(1); break;
            case F3      : switchTerminal(2); break;
            
            default:
                if (keyCodes[scancode] == '\b' && keyboard_buffer_idx<= 0)
                {
                    break;
                }
                
                if (scancode == L_SCAN_CODE && ctrlLock){ clear();} // special case for clearing screen
                
                else if (shiftLock && capsLock) { 

                    putc(shiftCapsCodes[scancode]); 
                    add_char_to_keyboard_buffer(shiftCapsCodes[scancode]);

                    }
                else if (shiftLock) { 
                    putc(shiftCodes[scancode]);
                    add_char_to_keyboard_buffer(shiftCodes[scancode]);

                    }
                else if (capsLock ) { 
                    putc(capsCodes[scancode]);
                    add_char_to_keyboard_buffer(capsCodes[scancode]);
                    
                    }
                                    
                else { 
                    putc(keyCodes[scancode]);
                    add_char_to_keyboard_buffer(keyCodes[scancode]);
                    
                    }
        }
        testKey = keyCodes[scancode];// used to run tests based on key pressed
    }

    // If buffer is full terminal will not write or accept new characters until backspace used
    else if(scancode == BACKSPACE){
        putc(keyCodes[scancode]);
        add_char_to_keyboard_buffer('\b');
    }
    // If enter is pressed with a full buffer, last character is replaced with \n 
    else if(scancode == ENTER_SCAN){ 
        putc(keyCodes[scancode]);
        // terminalRead(dummy_fd, keyboard_buffer, KEYBOARD_BUFFER_SIZE);
        // testKey = keyCodes[scancode];// used to run tests based on key pressed
        return_from_sys_write = 1;

    }
    

    send_eoi(IRQ_KEYBOARD);//  signaling end of keyboard interrupt handling
    restore_flags(flags);// restore flags after handling Interrupt
    sti(); // enable interrupts again
}

/* add_char_to_keyboard_buffer  
 *   DESCRIPTION: add pressed keys to keyboard buffer
 *   INPUTS: none
 *   RETURN VALUE: none */
void add_char_to_keyboard_buffer(char key){

    if(key == '\b'){
        keyboard_buffer_idx--;
        if(keyboard_buffer_idx < 0){
            keyboard_buffer_idx = 0;
        }
        return;
    }

    keyboard_buffer[keyboard_buffer_idx] = key;
    keyboard_buffer_idx++;

    if(key == '\n'){ // signal to copy keyboard buffer into terminal buffer
        return_from_sys_write = 1;
    }
};
