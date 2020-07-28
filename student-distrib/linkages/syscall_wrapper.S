#define ASM 1

.text

.globl syscall_handler

# syscall_handler
# Inputs: Args passed in via registers, listed below
# Outputs: none
# Returns: -1 if failed, status/error code otherwise
# Function: dispatches system calls */
syscall_handler:
#   EAX - SYSCALL number
#   EBX - 1st arg
#   ECX - 2nd arg
#   EDX -3rd arg
#

    # save all registers to the stack
    cli

    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushfl

    # add args onto stack
    pushl %EDX
    pushl %ECX
    pushl %EBX

    cmpl $1, %eax     # EAX contains syscall number
    jl   invalid_input    # if syscall number is <1 or >10 then it's invalid_input
    cmpl $10, %eax
    jg   invalid_input

    sti
    call *system_calls(,%eax,4)
    cli
    
    jmp leave_sys_call
    
invalid_input:
    # return error via %eax
    movl $-1, %eax
    
leave_sys_call:
    # remove args need to move esp by 3 -> 12
    addl $12, %esp
    
    # restore registers and return
    popfl
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    
    sti
    iret 
    
# Jump Table of System Calls (calls range from 1-10)
system_calls:
    .long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
