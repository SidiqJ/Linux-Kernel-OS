#include "pcb.h"

// Terminal Operations Pointers
ops_t stdInOps  ={terminalOpen, terminalRead, invalidOP    , terminalClose};
ops_t stdOutOps ={terminalOpen, invalidOP2  , terminalWrite, terminalClose};

int activeProccess[MAX_PCB] = {NULL,NULL,NULL,NULL,NULL,NULL};
int currentProccess = 0;
int processPerTerminal[MAX_TERMINAL_NUMBER][1] = {{0},{0},{0}};

int pid_running_in_terminal[3] = {-1,-1,-1};

/* initPCB
 * Inputs: pcb pointer and process id
 * Outputs: none
 * Function: initializes a new PCB struct */
void initPCB(pcb_t* pcb, int pid){
    int i;
    // FD initilization
    for (i = 0; i < MAX_FD; i++)
    {
        // pcb-> fdArray[i].operations = NULL;
        pcb->fdArray[i].inode      = NULL;
        pcb->fdArray[i].filePos    = NULL;
        pcb->fdArray[i].inUse      = 0;
    }
        pcb->fdArray[FD_STDIN].operations = stdInOps;
        pcb->fdArray[FD_STDIN].inUse      = 1;
        
        pcb->fdArray[FD_STDOUT].operations = stdOutOps;
        pcb->fdArray[FD_STDOUT].inUse      = 1;
    
    // PCB initilization
    pcb->processID   = pid;
    // if(pid){ 
    pcb->processESP  = NULL;
    pcb->processEBP  = NULL;
    // }
    // Parent Properties
    pcb->parentID    = NULL;
    // status
    pcb->status      = NULL;
    // vidmap
    pcb->vidMap_flag = 0;
    // printf("running in terminal: %d\n", currentTerminal);
    pcb->running_in_terminal_num = currentTerminal;
    // printf("stored running in terminal: %d\n", pcb->running_in_terminal_num);
    // printf("before:%d\n",processPerTerminal[currentTerminal][0]);
    processPerTerminal[currentTerminal][0]++;
    // printf("after:%d\n",processPerTerminal[currentTerminal][0]);
    
}

/* getUnusedProcessID
 * Inputs: none
 * Outputs: none
 * Returns: available process ID, or -1 if no process ID's available
 * Function: Gets free process ID, if one exists */
int getUnusedProcessID(){
    int i;
    for (i = 0; i < MAX_PCB; i++)
    {
        if (!activeProccess[i]){ return i; }
    }
    return -1;
}

/*  getPcbById
 * Inputs: process ID
 * Outputs: none
 * Returns: pointer to new PCB
 * Function: Get pointer to PCB for input process ID */
pcb_t* getPcbById(int pid){
    // processIDX start at 0, also we want the top of the block => hence the offset
    return (pcb_t*)(PCB_ADDR - PCB_8KB*(pid +1));
}

/*  getCurrentPCB
 * Inputs: none
 * Outputs: none
 * Returns: pointer to current PCB
 * Function: Get pointer to current PCB */
pcb_t*  getCurrentPCB(){
    pcb_t* pcb;
    asm volatile (
        "andl %%esp, %%eax;"
        : "=r" (pcb) /* output to current_pcb*/
        : "r" (PCB_MASK) /* input is the mask to get the pcb */
     );
    return pcb;
}

/*  invalidOP
 * Function: dummy placeholder function for file operations table */
int invalidOP(int32_t fd, const void * buf, int32_t nbytes){

     return -1;
}

/*  invalidOP
 * Function: dummy placeholder function for file operations table */
int invalidOP2(int32_t fd, void * buf, int32_t nbytes){

     return -1;
}
