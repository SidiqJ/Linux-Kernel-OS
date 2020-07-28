#include "interruptHandlers.h"

// Interrupt and System Call handler functions

// Timer chip interrupt handler
void IRQ0(){
    unsigned long flags;
    cli_and_save(flags); // disable Interrupts and save flags before handling Interrupt
    pit_scheduling();
    send_eoi(0x00);
    restore_flags(flags);// restore flags after handling Interrupt
    sti(); // enable interrupts again
}

// Slave PIC interrupt handler
void IRQ2(){
    printf("IRQ2: Cascade to slave");
    while(1){}
}
