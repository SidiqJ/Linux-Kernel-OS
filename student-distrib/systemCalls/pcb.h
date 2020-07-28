#ifndef PCB_H__
#define PCB_H__

#include "../types.h"
#include "../devices/terminal.h"

#define FD_STDIN  0
#define FD_STDOUT 1
#define MAX_PCB   6 // 6 processes max, but only 4/terminal
#define PCB_ADDR  0x800000 // PCB starting memory address
#define PCB_8KB   0x2000
#define PCB_MASK  0xFFFFE000
#define MAX_FD    8


int currentProccess, activeProccess[MAX_PCB];
int processPerTerminal[MAX_TERMINAL_NUMBER][1];

int pid_running_in_terminal[3];

typedef struct Operations
{   
    int (*openFD )(const uint8_t* filename);
	int (*readFD )(int32_t fd, void* buf, int32_t nbytes);
	int (*writeFD)(int32_t fd, const void* buf, int32_t nbytes);
	int (*closeFD)(int32_t fd);
} ops_t;

typedef struct FileDescriptor
{
    ops_t       operations;
    uint32_t    inode;
    uint32_t    filePos;
    uint32_t    inUse;
} fileDesc_t;

typedef struct ProcessControlBlock
{   // File Descriptor Array
    fileDesc_t  fdArray[MAX_FD];
    // Process Properties
    uint32_t    processID;
    uint32_t    processESP;
    uint32_t    processEBP;

    uint32_t    parentESP;
    uint32_t    parentEBP;      

    // Parent Properties
    uint32_t    parentID;
    // status
    uint32_t    status;
    // arguments
    uint8_t     arguments[KEYBOARD_BUFFER_SIZE];
    // vidmap flag
    uint32_t    vidMap_flag; 
    // running in terminal
    uint32_t    running_in_terminal_num;

} pcb_t;

extern void initPCB(pcb_t* pcb, int pid);
extern int  getUnusedProcessID ();
extern pcb_t*  getPcbById   (int pid);
extern pcb_t*  getCurrentPCB();
extern int invalidOP(int32_t fd, const void * buf, int32_t nbytes);
extern int invalidOP2(int32_t fd, void * buf, int32_t nbytes);


#endif

