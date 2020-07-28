#include "rtc.h"

char oldVal;
unsigned long flags;

int rtc_interrupt_flag = 0;

/* initRTC
 * Inputs: None
 * Outputs: none
 * Function: Initializes RTC and enables its IRQ line */
void initRTC(){

    cli_and_save(flags); // Works without this and sti -> do we still need

    disable_irq(IRQ_RTC); // Disable IRQ interrupt for init

    // Set default freq to 1024Hz
    changeRtcFreq(MAX_FREQ);

    // Turn on IRQ8
    outb(PORT_B, RTC_PORT);
    oldVal = inb(CMOS_PORT);
    outb(PORT_B, RTC_PORT);

    // Turn on bit 6 of register B
    outb(oldVal | MASK_40 ,CMOS_PORT); 

    rtc_test_Flag = 0;

    enable_irq(IRQ_RTC);

    restore_flags(flags);
    sti();

};

/* rtcHandler
 * Inputs: None
 * Outputs: none
 * Function: Handles RTC interrupt */
void rtcHandler(){

    cli_and_save(flags); // Works without this and sti -> do we still need
    
    //test_interrupts();
    if (rtc_test_Flag){
    putc('1');} // Display 1 as test character
    
    // Throw away contents of Register C so interrupts reccur 
    outb(PORT_C, RTC_PORT);
    inb(CMOS_PORT);

    send_eoi(IRQ_RTC);

    // Clear interrupt flag for RTC read func.
    rtc_interrupt_flag = 0;

    restore_flags(flags);
    sti();

};

/* rtcOpen
 * Inputs: filename
 * Outputs: none
 * Function: Sets the RTC interrupt frequency to 2Hz */
int rtcOpen(const uint8_t* filename){

    cli_and_save(flags);
    
    changeRtcFreq(DEFAULT_2HZ);

    restore_flags(flags);
    sti();

    return 0;
};

/* rtcRead
 * Inputs: file descriptor, pointer to buffer with data, number of bytes to be written
 * Outputs: none
 * Returns: always 0
 * Function: Wait for interrupt to occur */
int rtcRead(int32_t fd, void* buf, int32_t nbytes){

    rtc_interrupt_flag = 1;
    
    while(rtc_interrupt_flag == 1){
        // wait for interrupt handler to occur
    }

    return 0; 

};

/* rtcWrite
 * Inputs: file descriptor, pointer to buffer with data, number of bytes to be written
 * Outputs: none
 * Returns: -1 if invalid frequency, 4 (bytes written) if freq can be/was changed
 * Function: Change the RTC frequency */
int rtcWrite(int32_t fd, const void* buf, int32_t nbytes){

    int32_t new_frequency = *((int32_t * )buf); // cast buffer to int and dereference to get data
    
    // check if frequency is in range (2Hz - 1024Hz) 
    if((new_frequency < DEFAULT_2HZ) || (new_frequency > MAX_FREQ) || buf == NULL){
        return -1;
    }
    // check if frequency is a power of two
    uint32_t freq_check = new_frequency;

    // count set bits
    int set_bits = 0;
    while(freq_check){
        set_bits += freq_check & 1;
        freq_check >>= 1;
    }

    // not a power of two return -1
    if(set_bits != 1){
        return -1;
    }  
    cli_and_save(flags);
    changeRtcFreq(new_frequency);
    restore_flags(flags);
    sti();    
    return 4; // successfully wrote 4 bytes

};

/* rtcClose
 * Inputs: file descriptor, pointer to buffer with data, number of bytes to be written
 * Outputs: none
 * Returns: always 0
 * Function: Closes the file descriptor */
int rtcClose(int32_t fd){

    return 0;
};

/* changeRtcFreq
 * Inputs: desired new frequency
 * Outputs: none
 * Function: Changes the RTC interrupt frequency */
void changeRtcFreq(int newFreq){

    char rate;

    /* change frequency, each case represents a freq in Hz and assigns
    *  rate to the correct binary value as indicated in the datasheet
    */
    switch (newFreq)
    {
    case 2: // 2Hz
        rate = 0x0F; // Bit value for chip
        break;

    case 4: // 4Hz
        rate = 0x0E; // Bit value for chip
        break;

    case 8: // 8Hz
        rate = 0x0D; // Bit value for chip
        break;

    case 16: // 16Hz
        rate = 0x0C; // Bit value for chip
        break;

    case 32: // 32Hz
        rate = 0x0B; // Bit value for chip
        break;

    case 64: // 64Hz
        rate = 0x0A; // Bit value for chip
        break;

    case 128: // 128Hz
        rate = 0x09; // Bit value for chip
        break;

    case 256: // 256Hz
        rate = 0x08; // Bit value for chip
        break;

    case 512: // 512Hz
        rate = 0x07; // Bit value for chip
        break;
    
    case 1024: // 1024Hz
        rate = 0x06; // Bit value for chip
        break;

    }

    outb(PORT_A, RTC_PORT); // set index to register A, disable NMI
    oldVal = inb(CMOS_PORT); // get current value
    outb(PORT_A, RTC_PORT); // reset index to A
    outb((oldVal & RATE_MASK) | rate, CMOS_PORT); // write new rate to A

};
