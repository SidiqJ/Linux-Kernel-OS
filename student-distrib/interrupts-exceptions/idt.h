#pragma once

#include "../x86_desc.h"
#include "../lib.h"
#include "interruptHandlers.h"
#include "exceptionHandlers.h"
#include "../linkages/isr_wrapper.h"
#include "../linkages/syscall_wrapper.h"


#define SYS_CALL 0x80

// initializes the idt
extern void init_idt();
