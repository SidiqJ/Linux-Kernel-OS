#include "terminal.h"
#include "../systemCalls/systemCalls.h"



/* initTerminals
 * Inputs: filename
 * Outputs: none
 * Function: Initializes the terminal structs that keep track of terminal data for switching */
void initTerminals(){
    int i;

    currentTerminal = 0;
    clear();

    // Set up each terminal struct
    for(i=0;i<MAX_TERMINAL_NUMBER;i++){
        terminal_info_t terminal;
        terminal.terminal_num = i;
        terminal.in_use = 0;
        terminal.x = 0;
        terminal.y = 0;
        terminal.keyboard_buffer_idx = 0;
        available_terminals[i] = terminal;
        memcpy((void *) (VIDEO + _4KB*(i+1)), (void*) VIDEO, _4KB);
    }

}

/* switchTerminal
 * Inputs: index of terminal you want to switch into
 * Outputs: none
 * Function: Switches between terminals */
void switchTerminal(int terminalIndex){
    // cli();
    // Create new terminal
    if(!available_terminals[terminalIndex].in_use){ 

        available_terminals[terminalIndex].in_use = 1;
        
        //switch screens
        switchScreen(&available_terminals[currentTerminal],&available_terminals[terminalIndex]);

        pcb_t * current_process_pcb = getPcbById(currentTerminal);
        // watch out for this when only runinng a process in a single shell

        // if (terminalIndex != currentTerminal)
        // {
        // pcb_t * old_process_pcb = getPcbById(currentTerminal);
        // }
        

        /*
        Terminal0: create Shell (pid 0) and save  ESP AND EBP are SAVED
        Terminal0: counter (pid 1) - DEFAULT ESP AND EBP = 0
        Switch terminal 1: create shell (pid 2) and save ESP AND EBP

        Switch to terminal 0 => BREAKS :(
            */


        currentTerminal = terminalIndex;
        // execute shell
        printf("Creating Terminal %d\n", terminalIndex);
        send_eoi(IRQ_KEYBOARD);//  signaling end of keyboard interrupt handling

        // save esp ebp 
        asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1"
        :"=r" (current_process_pcb->processESP), "=r"(current_process_pcb->processEBP)
        :
        :"memory"
        );

        execute((uint8_t*)"shell");
        
        return;

    } else { // Terminal exists. Switch to it

        send_eoi(IRQ_KEYBOARD);//  signaling end of keyboard interrupt handling
        
        // Save esp and ebp for process that is being switched out of
        pcb_t * current_process_pcb = getPcbById(currentProccess);

        asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1"
        :"=r" (current_process_pcb->processESP), "=r"(current_process_pcb->processEBP)
        :
        :"memory"
        );
        
        switchScreen(&available_terminals[currentTerminal],&available_terminals[terminalIndex]);

        currentTerminal = terminalIndex;
        
        // Update currently running process 
        currentProccess = pid_running_in_terminal[terminalIndex];

        // Do task switch
        pcb_t * next_process_pcb = getPcbById(currentProccess);

        tss.esp0 = PCB_ADDR - PCB_8KB*(currentProccess) - 4; // Subt 4 bytes per DOCS
        tss.ss0 = KERNEL_DS;

        create_page_dir_entry(currentProccess*_4MB + _8MB);

        asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp"
        :
        :"r"(next_process_pcb->processESP), "r"(next_process_pcb->processEBP)
        :"memory"
    );
    
    }
    // sti();
    
}

/* switchScreen
 * Inputs: filename
 * Outputs: none
 * Function: Remaps video memory and transfers cursor data. Also copies over buffer info */
void switchScreen(terminal_info_t * currentTerminal, terminal_info_t * newTerminal){
    
    // save cursor coords 
    currentTerminal->x = screen_x;
    currentTerminal->y = screen_y;
    currentTerminal->keyboard_buffer_idx = keyboard_buffer_idx;

    // save keyboard and terminal buffers
    strcpy((int8_t*) currentTerminal->keyboard_buffer, (int8_t*) keyboard_buffer);
    strcpy((int8_t*) currentTerminal->terminal_buffer, (int8_t*) terminal_buffer);

    /* Table:
            T0 => B9000
            T1 => BA000
            T2 => BB000
    */
    //save screen
    memcpy((void *) (VIDEO + _4KB*(currentTerminal->terminal_num+1)), (void*) VIDEO, _4KB);
    //Updates position and cursor
    screen_x = newTerminal->x;
    screen_y = newTerminal->y;
    setCursor(screen_x, screen_y);
    
    // write
    memcpy((void *) VIDEO, (void*) (VIDEO + _4KB*(newTerminal->terminal_num+1)), _4KB);
    
    // restore keyboard and terminal buffers
    strcpy((int8_t*) keyboard_buffer, (int8_t*) newTerminal->keyboard_buffer);
    strcpy((int8_t*) terminal_buffer, (int8_t*) newTerminal->terminal_buffer);
    keyboard_buffer_idx = newTerminal->keyboard_buffer_idx;
    
}

/* terminalOpen
 * Inputs: filename
 * Outputs: none
 * Function:  */
int terminalOpen(const uint8_t* filename){

    return 0;
};

/* terminalRead
 * Inputs: file descriptor, pointer to buffer with data, number of bytes to be written
 * Outputs: none
 * Returns: always 0
 * Function: Read contents of keyboard buffer into terminal buffer */
int terminalRead(int32_t fd, void* buf, int32_t nbytes){

    if(buf == NULL || nbytes<0){
        return -1;
    }

    while (return_from_sys_write == 0){/* Wait for user to press enter to copy data*/
        // send_eoi(IRQ_KEYBOARD);
        // send_eoi(0x00);
    }
   
    passed_in_buffer = (char *) buf;

    clear_buffer(CLR_TERMINAL_BUFFER); 

    int i; // use as index and also to keep track of bytes written for return

    for(i=0; i<KEYBOARD_BUFFER_SIZE - 1 ;i++){ // only read 127 bytes max
        // transfer contents of keyboard buffer over
        passed_in_buffer[i] = keyboard_buffer[i];
        if(keyboard_buffer[i] == '\n'){
            break;
        }
    }

    // // make last entry new line character
    if(i == KEYBOARD_BUFFER_SIZE){
        passed_in_buffer[i] = '\n';
    }


    // clear keyboard buffer
    buffer_read_bytes= i+1;

    //printf("Bytes:%d\n",i);
    
    return_from_sys_write = 0;
    
    clear_buffer(CLR_KEYBOARD_BUFFER); 
    // printf("Passed in:%s", passed_in_buffer);
    return i + 1; 
};

/* terminalWrite
 * Inputs: file descriptor, pointer to buffer with data, number of bytes to be written
 * Outputs: none
 * Returns: 
 * Function: Writes buffer data onto screen */
int terminalWrite(int32_t fd, const void* buf, int32_t nbytes){ // no limit to nbytes

    if(buf == NULL){
        return -1;
    }

    char * passed_in_buffer = (char *) buf;

    // printf("Passed in:%s\n", passed_in_buffer);

    int i;
    int bytes_written = 0; 

    for(i=0;i<nbytes;i++){
        if(passed_in_buffer[i] != 0x0){  // Check for null character 0x0
            putc(passed_in_buffer[i]); // dont print a null 
            bytes_written++;
        }
    }

    return bytes_written; // return number of bytes written 

};

/* terminalClose
 * Inputs: file descriptor, pointer to buffer with data, number of bytes to be written
 * Outputs: none
 * Returns: always 0
 * Function: Closes the file descriptor */
int terminalClose(int32_t fd){
    return 0;

};

/* clear_buffer
 * Inputs: none
 * Outputs: none
 * Function: Clears keyboard or terminal buffer */
void clear_buffer(int which_buffer){

    int i;
    if (which_buffer == CLR_KEYBOARD_BUFFER)
    {
        for(i=0; i<KEYBOARD_BUFFER_SIZE; i++){
        keyboard_buffer[i] = 0x0; // set all entries to null character
        }
         keyboard_buffer_idx = 0;
    }
    else if (which_buffer == CLR_TERMINAL_BUFFER)
    {
        for(i=0; i<KEYBOARD_BUFFER_SIZE; i++){
        passed_in_buffer[i] = 0x0; // set all entries to null character
        }
        keyboard_buffer_idx = 0;
    }

};

/* setCursor
 * Inputs: Coordinates : X and Y
 * Outputs: none
 * Function: Updates cursor location based on given parameters */
void setCursor (int x, int y){
    uint16_t position = y * SCREEN_WIDTH + x;
    // select lower byte location register and send in position
    outb(LOCATION_LOW, CURSOR_PORT);
    outb((uint8_t) position, CURSOR_DATA);

    // select higher byte location register & send the higher byte(shift right BY 8 BITS) of the position
    outb(LOCATION_HIGH, CURSOR_PORT);
    outb((uint8_t) ((position>>8)), CURSOR_DATA); 
};
