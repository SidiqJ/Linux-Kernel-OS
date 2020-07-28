#include "idt.h"

/* init_idt;
 * Inputs: none
 * Return Value: none
 * Function: Initializes and populates the IDT */
void init_idt(){

    // load idt
    lidt(idt_desc_ptr);

    int idt_idx;

    // initialize each entry in idt
    for (idt_idx = 0; idt_idx < NUM_VEC; idt_idx++){
        
        idt[idt_idx].seg_selector = KERNEL_CS;
        idt[idt_idx].reserved4 = 0;
        idt[idt_idx].reserved3 = 0;
        idt[idt_idx].reserved2 = 1;
        idt[idt_idx].reserved1 = 1;
        idt[idt_idx].size = 1;
        idt[idt_idx].reserved0 = 0;

        // set exceptions to use trap gates
        if(idt_idx < 32){
            idt[idt_idx].reserved3 = 1;
        }
        
        // Set DPL to 0 for hardware interrupt handlers and exception handlers
        idt[idt_idx].dpl = 0; 
        
        // Set DPL to 3 for system call handler
        if(idt_idx == SYS_CALL){
            idt[idt_idx].reserved3 = 1;
            idt[idt_idx].dpl = 3;  
        }
        
        idt[idt_idx].present = 1;

        // disable non used IRQ vectors and APIC vectors
        if(idt_idx > 0x22 && idt_idx != SYS_CALL && idt_idx != 0x28){
            idt[idt_idx].present = 0;
        }
        
    }
    // Defined by Intel   
    SET_IDT_ENTRY(idt[0x00], divideError);
    SET_IDT_ENTRY(idt[0x01], reserved);
    SET_IDT_ENTRY(idt[0x02], NMINT);
    SET_IDT_ENTRY(idt[0x03], breakpoint);
    SET_IDT_ENTRY(idt[0x04], overflow);
    SET_IDT_ENTRY(idt[0x05], boundRangeExceeded);
    SET_IDT_ENTRY(idt[0x06], invalidOpcode);
    SET_IDT_ENTRY(idt[0x07], deviceNA);
    SET_IDT_ENTRY(idt[0x08], doubleFault);
    SET_IDT_ENTRY(idt[0x09], segmentOverrun);
    SET_IDT_ENTRY(idt[0x0A], invalidTSS);
    SET_IDT_ENTRY(idt[0x0B], segmentNotPresent);
    SET_IDT_ENTRY(idt[0x0C], segmentFault);
    SET_IDT_ENTRY(idt[0x0D], generalProtection);
    SET_IDT_ENTRY(idt[0x0E], pageFault);
    SET_IDT_ENTRY(idt[0x10], mathFault);
    SET_IDT_ENTRY(idt[0x11], alignmentCheck);
    SET_IDT_ENTRY(idt[0x12], machineCheck);
    SET_IDT_ENTRY(idt[0x13], floatingPointException);
    // Device interrupt handlers
    SET_IDT_ENTRY(idt[0x20], timer_handler); // Timer Chip
    SET_IDT_ENTRY(idt[0x21], keyboard_handler); // Keyboard
    SET_IDT_ENTRY(idt[0x22], slave_pic); // Slave PIC
    SET_IDT_ENTRY(idt[0x28], rtc); // RTC
    // System call
    SET_IDT_ENTRY(idt[SYS_CALL], syscall_handler);//systemCall);

}
