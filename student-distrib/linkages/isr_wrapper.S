.text

.globl timer_handler, keyboard_handler, slave_pic, rtc

# Assembly linkages for handling interrupt service routines

# ----------------------------------------------------------------------------- #
# timer_handler
# Inputs: None
# Outputs: None
# Function: Saves current state and passes execution to timer handler
# ----------------------------------------------------------------------------- #
timer_handler:
    # Push and Restore  all regs & flags when an interrupt gets triggered
    pushal  
    pushfl  
    call IRQ0 # call IRQ0 handler
    popfl
    popal
    iret

# ----------------------------------------------------------------------------- #
# keyboard_handler
# Inputs: None
# Outputs: None
# Function: Saves current state and passes execution to keyboard handler
# ----------------------------------------------------------------------------- #
keyboard_handler:
    pushal 
    pushfl
    call keyboardHandler # call Keyboard handler that echo input to screen
    popfl
    popal
    iret

# ----------------------------------------------------------------------------- #
# slave_handler
# Inputs: None
# Outputs: None
# Function: Saves current state and passes execution to slave pic
# ----------------------------------------------------------------------------- #
slave_pic:
    pushal 
    pushfl
    call IRQ2 # call IRQ2 handler
    popfl
    popal
    iret

# ----------------------------------------------------------------------------- #
# rtc
# Inputs: None
# Outputs: None
# Function: Saves current state and passes execution to rtc
# ----------------------------------------------------------------------------- #
rtc:
    pushal 
    pushfl
    call rtcHandler # call RTC handler
    popfl
    popal
    iret

.end
