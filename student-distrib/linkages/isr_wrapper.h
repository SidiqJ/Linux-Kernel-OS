#ifndef _ISR_WRAPPER_H
#define _ISR_WRAPPER_H

#include "../devices/keyboard.h"
#include "../devices/rtc.h"

// assembly linkages for interrupts
extern void timer_handler();
extern void keyboard_handler();
extern void slave_pic();
extern void rtc();

#endif
