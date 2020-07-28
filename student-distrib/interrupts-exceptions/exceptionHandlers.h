#include "../lib.h"
#include "../systemCalls/systemCalls.h"


#define MINUS_ONE -1

// exception handler functions for each exception
extern void divideError();
extern void reserved();
extern void NMINT();
extern void breakpoint();
extern void overflow();
extern void boundRangeExceeded();
extern void invalidOpcode();
extern void deviceNA();
extern void doubleFault();
extern void segmentOverrun();
extern void invalidTSS();
extern void segmentNotPresent();
extern void segmentFault();
extern void generalProtection();
extern void pageFault();
extern void intelReserved();
extern void mathFault();
extern void alignmentCheck();
extern void machineCheck();
extern void floatingPointException();
