#ifndef SCHEDULING_H__
#define SCHEDULING_H__
#define _8KB 0x10000
#include "../systemCalls/systemCalls.h"
#include "../devices/terminal.h"
#include "../paging/paging.h"
// #include "../systemCalls/pcb.h"

#define MODE_3  0x36
#define CMD_REG 0x43

#define PIT_VALUE   298295
#define LOW_BYTE_MASK   0xFF
#define HIGH_BYTE_SHIFT 8
#define CHANNEL_0   0x40


int currentIndex;

int pit_interrupt_count;

void init_pit();
void pit_scheduling();

#endif /* SCHEDULING_H__*/
