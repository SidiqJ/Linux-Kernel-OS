#include "scheduling.h"

/* init_pit
 * Inputs: none
 * Outputs: none
 * Function: initializes the pit 
 */

void init_pit() {
    // 0x40 - Channel 0 data port
    // 0x41 - Channel 1 data port
    // 0x42 - Channel 2 data port
    // 0x43 - Mode/Command register (write only)

    // bits 6 and 7 - select channel
    // bits 4 and 5 - access mode
    // bits 1 to 3 - operating mode
    // bit 0 - bcd/binary mode

    // want to set mode 3 (square wave generator) to command register (0x43)
    // 00 11 011 0 -> 0x36
    outb(MODE_3, CMD_REG);
    // need to set frequency using the formula 11931820 / x = f
    // set frequency to 25 Hz -> x = 477272.8 -> 477273 ... must be sent to PIT split into a high and low byte
    // sends low byte
    //outb(477273 & 0xFF, 0x40);

    outb(PIT_VALUE & LOW_BYTE_MASK, CHANNEL_0);
    // sends high byte
    outb((PIT_VALUE >> HIGH_BYTE_SHIFT), CHANNEL_0);

    //currentIndex = -1;
    currentIndex = 2; // start scheduler at terminal 2

    pit_interrupt_count = 0;

    // enable PIT (generates IRQ 0)
    enable_irq(0x00);
}

/* pit_scheduling
 * Inputs: none
 * Outputs: none
 * Function: pit handler function, runs processes in background/foreground 
 */

void pit_scheduling() {
    // uint32_t addr;
    // uint32_t next_index;
    
    // Set up three shells
    if(pit_interrupt_count<MAX_TERMINAL_NUMBER){
        pit_interrupt_count++;
        send_eoi(0x00);
        switchTerminal(pit_interrupt_count-1);
    }

    // finds the next scheduled proccess (round-robin)
    // next_index = (currentIndex + 1) % MAX_TERMINAL_NUMBER;

    // // // switch(next_index) {
    // // //     case 0:
    // // //         page_table_index = 0xB9;
    // // //         video_address = 0xB9000;
    // // //         break;
    // // //     case 1:
    // // //         page_table_index = 0xBA;
    // // //         video_address = 0xBA000;
    // // //         break;
    // // //     case 2:
    // // //         page_table_index = 0xBB;
    // // //         video_address = 0XBB000;
    // // //         break;
    // // // }

    
    // // get current PCB
    // pcb_t * current_process_pcb = getPcbById(pid_running_in_terminal[currentIndex]);


    // asm volatile(
    //     "movl %%esp, %0;"
    //     "movl %%ebp, %1"
    //     :"=r" (current_process_pcb->processESP), "=r"(current_process_pcb->processEBP)
    //     :
    //     :"memory"
    //     );


    // //currentTerminal = next_index;
        
    // // Update currently running process 
    // //currentProccess = pid_running_in_terminal[next_index];

    // // gets the next pcb
    // pcb_t * next_process_pcb = getPcbById(pid_running_in_terminal[next_index]);

    // tss.esp0 = PCB_ADDR - PCB_8KB*(pid_running_in_terminal[next_index]) - 4; // Subt 4 bytes per DOCS
    // tss.ss0 = KERNEL_DS;

    // set_video_memory_lib(next_index);

    // create_page_dir_entry(pid_running_in_terminal[next_index]*_4MB + _8MB);

    //     asm volatile(
    //     "movl %0, %%esp;"
    //     "movl %1, %%ebp"
    //     :
    //     :"r"(next_process_pcb->processESP), "r"(next_process_pcb->processEBP)
    //     :"memory"
    // );

    // send_eoi(0x00);
    
}

