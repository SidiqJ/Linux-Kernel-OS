#include "exceptionHandlers.h"

// Exception handler functions
void divideError(){
    printf("0 - Fault:#DE Divide Error \nSource: DIV and IDIV\n");
    halt(MINUS_ONE);
    //while(1){}
}
void reserved(){
    printf("1 - Fault/Trap:#DB RESERVED \nSource: For intel user only\n");
    halt(MINUS_ONE);
    //while(1){}
}
void NMINT(){
    printf("2 - Interrupt: NMI intterupt \nSource: Nonmaskable external interrupt\n");
    halt(MINUS_ONE);
    //while(1){}
}
void breakpoint(){
    printf("3 - Trap:#BP Breakpoint \nSource: INT 3 instruction\n");
    halt(MINUS_ONE);
    //while(1){}
}
void overflow(){
    printf("4 - Trap:#OF Overflow \nSource: INTO instruction\n");
    halt(MINUS_ONE);
    //while(1){}
}
void boundRangeExceeded(){
    printf("5 - Fault:#BR Bound Range Exceeded \nSource: Bound instruction\n");
    halt(MINUS_ONE);
    //while(1){}
}
void invalidOpcode(){
    printf("6 - Fault:#UD Invalid/Undefined Opcode \nSource: UD2 instruction or reserved opcode\n");
    halt(MINUS_ONE);
    //while(1){}
}
void deviceNA(){
    printf("7 - Fault:#NM Device Not Available \nSource: Floating-point or WAIT/FWAIT instruction\n");
    halt(MINUS_ONE);
    //while(1){}
}
void doubleFault(){
    printf("8 - Abort:#DF Double fault \nSource: any instruction that generates an exception/NMI/INTR\n");
    halt(MINUS_ONE);
    //while(1){}
}
void segmentOverrun(){
    printf("9 - Fault: Coprocessor SegmentOverrun(reserved) \nSource: floating-point instruction\n");
    halt(MINUS_ONE);
    //while(1){}
}
void invalidTSS(){
    printf("10 - Fault:#TS invalid TSS \nSource: Task switch or TSS access\n");
    halt(MINUS_ONE);
    //while(1){}
}
void segmentNotPresent(){
    printf("11 - Fault:#NP Segment Not Present \nSource: Loading segment registers/accessing system segments\n");
    halt(MINUS_ONE);
    //while(1){}
}
void segmentFault(){
    printf("12 - Fault:#SS Stack Segment Fault \nSource: Stack operations and SS register loads\n");
    halt(MINUS_ONE);
    //while(1){}
}
void generalProtection(){
    printf("13 - Fault:#GP General Protection \nSource: any memory refrence and other protection checks\n");
    halt(MINUS_ONE);
    //while(1){}
}
void pageFault(){
    int addr;
    asm volatile( 
        "movl %%cr2, %0;"
        :"=r"(addr)
        :
        :"cc"
        );
        // Display address that prog tries to access
        printf("14 - Fault:#PF Page Fault \nSource: %x\n", addr);

       halt(MINUS_ONE);
    //while(1){}
}
void mathFault(){
    printf("16 - Fault:#MF x87 FPU Floating-point Error (Math Fault) \nSource: x87 FPU Floating-point or WAIT/FWAIT instruction\n");
    halt(MINUS_ONE);
    //while(1){}
}
void alignmentCheck(){
    printf("17 - Fault:#AC Alignment Check \nSource: Any data reference in memory\n");
    halt(MINUS_ONE);
    //while(1){}
}
void machineCheck(){
    printf("18 - Fault:#MC Machine Check \nSource: Error codes(if any) and source are model dependent\n");
    halt(MINUS_ONE);
    //while(1){}
}
void floatingPointException(){
    printf("19 - Fault:#XF SIMD Floating-Point Exception \nSource: SSE/SSE2/SSE3 floating-point instructions\n");
    halt(MINUS_ONE);
    //while(1){}
}
