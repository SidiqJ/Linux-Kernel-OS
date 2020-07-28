#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xff; /* IRQs 0-7  */
uint8_t slave_mask = 0xff;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {

    // save flags & block interrupts
    unsigned long flags;
    cli_and_save(flags);
    // Mask all interrupts for both PICs
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);

    // Initialize the master
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA); // Normal EOI

    // Initialize the slave
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA); // Normal EOI
    
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
    
    enable_irq(SLAVE_IRQ);

    restore_flags(flags); // Was told to remove - check later
    sti();
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

    // Enable irq in master
    if(irq_num >= 0 && irq_num < NUM_OF_PORTS){
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    }
    // Enable irq in slave
    else {
        slave_mask &= ~(1 << (irq_num - NUM_OF_PORTS));
        outb(slave_mask, SLAVE_8259_DATA);
    }

}


/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {

    // Disable irq in master
    if(irq_num > -1 && irq_num < NUM_OF_PORTS){
        master_mask |= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    }
    // Disable irq in slave
    else {
        slave_mask |= ~(1 << (irq_num - NUM_OF_PORTS));
        outb(slave_mask, SLAVE_8259_DATA);
    }

}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

    if(irq_num >= NUM_OF_PORTS){
        // First send EOI from master, then from slave
        outb(((SLAVE_IRQ) | EOI), MASTER_8259_PORT);
        outb(((irq_num - NUM_OF_PORTS) | EOI), SLAVE_8259_PORT);
    }
    else {
        // EOI from master
        outb((irq_num | EOI), MASTER_8259_PORT);
    }
    
}
