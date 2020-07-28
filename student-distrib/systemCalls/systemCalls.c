#include "systemCalls.h"

ops_t rtcOps    ={rtcOpen          , rtcRead     , rtcWrite    , rtcClose};
ops_t fsDirOps  ={fs_open_directory, fs_read_directory, fs_write_directory, fs_close_directory};
ops_t fsOps     ={fs_open, fs_read , fs_write, fs_close};


/* halt
 * Inputs: status code
 * Returns: error code, or 0
 * Function: halts currently running process */
int32_t halt(uint8_t status){
     // figure out which terminal we are in and then grab that last PCB
     pcb_t* currentPCB = getPcbById(currentProccess); 
     
     // If base shell is halted, execute it again
     if (processPerTerminal[currentPCB->running_in_terminal_num][0] <=1)
     {    
          printf("You cannot exit the original shell\n");
          //update bookkeeping info about active processes
          activeProccess[currentProccess] = 0;
          pid_running_in_terminal[currentPCB->running_in_terminal_num] = -1;
          processPerTerminal[currentPCB->running_in_terminal_num][0]--;
          execute((uint8_t*)"shell");
     }



     currentPCB->status = status;
     // halt_status = (uint32_t) status;

     // prog exits from exception 
     if (status == 255){ // 255 exception triggered
          printf("exception occured... ");
          currentPCB->status = EXCEPTION_STATUS;
     }

     activeProccess[currentProccess] = 0;

     // restore paging
     create_page_dir_entry(currentPCB->parentID*_4MB + _8MB);
     currentProccess = currentPCB->parentID;

     pid_running_in_terminal[currentPCB->running_in_terminal_num] = currentProccess;

     // reclaim video memory
     if(currentPCB->vidMap_flag == 1){
          reclaim_video_memory();
     }

     // clear fd
     int fdIdx;
     for (fdIdx = 0; fdIdx < MAX_FD; fdIdx++)
     {
          if (currentPCB->fdArray[fdIdx].inUse && fdIdx>FD_STDOUT)
          {
               close(fdIdx);
          }
          
          // currentPCB->fdArray[fdIdx].operations = invalidOP;
          currentPCB->fdArray[fdIdx].inUse = 0;
     }

     // write parent process' info back to TSS
     tss.esp0 = currentPCB->parentESP;
     printf("Exitting process %d\n", processPerTerminal[currentPCB->running_in_terminal_num][0]);
     processPerTerminal[currentPCB->running_in_terminal_num][0]--;
     //printf("Program %d Exiting...\n", currentProccess + 1);

     // jump to execute return
     asm volatile(

          "movl %0,     %%eax;"
          "movl %1,     %%esp;"
          "movl %2,     %%ebp;"
          "jmp       IRET_OUT;"
          :
          : "r" (currentPCB->status), "r" (currentPCB->parentESP), "r" (currentPCB->parentEBP)
          :"eax"
     );
     
    return 0;
}

/* execute
 * Inputs: command to execute
 * Returns: -1 if execute call fails
 * Function: execute a process */
int32_t execute(const uint8_t * command){

     if(command == NULL || strlen((int8_t *)command)>(KEYBOARD_BUFFER_SIZE -1)){ 
          // printf("Command is Invalid\n");
          return -1;
     }

     int pid, argIdx, fnIdx =0;
     uint8_t elf_buffer[COMMAND_BUFF_SIZE];
     uint8_t entry_pt_buffer[COMMAND_BUFF_SIZE];
     uint8_t elf_magic_number[COMMAND_BUFF_SIZE] = {0x07F, 0x45, 0x4C, 0x46}; // ELF magic numbers

     /************* Parse Args ************ */

     // find start of filename -> end is taken care of read_dentry_by_name
     for(fnIdx=0;fnIdx<KEYBOARD_BUFFER_SIZE;fnIdx++){
          if(command[fnIdx] != ' '){
               break;
          }
     }

     uint8_t filename[KEYBOARD_BUFFER_SIZE];
     // copy filename over from command
     strcpy((int8_t*)filename, (int8_t*)(command+fnIdx));

     // find start of args
     for(; fnIdx<KEYBOARD_BUFFER_SIZE;fnIdx++){ 
          if(command[fnIdx] == ' '){
              break;
          }
     }
     for(argIdx = fnIdx; argIdx < KEYBOARD_BUFFER_SIZE; argIdx++){
          if(command[argIdx] != ' '){
               break;
          }
     }
          

     // check file validity. Return -1 if prog does not exist
     directory_entry_t dentry;

     if(read_dentry_by_name(filename, &dentry) == -1){
          // printf("Execute: Reading directory failed\n");
          return -1;
     }

     // file exists, check if executable. Read first 4 bytes and check if elf magic number matches
     read_data(dentry.inodeNum, 0, elf_buffer, COMMAND_BUFF_SIZE);
     if(strncmp((int8_t*) elf_magic_number, (int8_t*) elf_buffer, COMMAND_BUFF_SIZE) != 0){
          // printf("Execute: magic number:%x check failed\n", elf_buffer);
          return -1;
     }

     // find entry point into the program 4-byte unsinged int in bytes 24-27
     read_data(dentry.inodeNum, BUFF_READ_START, entry_pt_buffer, COMMAND_BUFF_SIZE);

     entry_pt = *((uint32_t*)entry_pt_buffer);

     /* ************ Create PCB ************ */

     // check if a process is available
     if((pid = getUnusedProcessID()) == -1){
          printf("Execute: pid get failed\n");
          return -1;
     }
     
     activeProccess[pid] = 1; // Turns on flag that there's a process running
     
     pcb_t* newPCB = getPcbById(pid);
     initPCB(newPCB, pid);
     // printf("init idx: %d\n",processPerTerminal[newPCB->running_in_terminal_num][0]);
     asm volatile(
        "movl %%ebp, %%ebx;"
        "movl %%esp, %%ecx;"
        "/*populate PCB with outputs*/"
        "/* b(EBX) contains EBP      C(ECI) contains stack pointer*/"
        : "=b" (newPCB->parentEBP), "=c" (newPCB->parentESP)
        :
        :"eax"
     );


     currentProccess = pid; // Update current proccess

     pcb_t* parentPCB;

     // set up link to parent process if from different terminal
     if(pid_running_in_terminal[newPCB->running_in_terminal_num]> -1){
          parentPCB = getPcbById(pid_running_in_terminal[newPCB->running_in_terminal_num]);
     }
     else { // if process is the base process on given terminal make its parent itself
          
          parentPCB = newPCB;
     }

     pid_running_in_terminal[newPCB->running_in_terminal_num] = pid; //top pid running in each terminal

     //pcb_t* parentPCB = getCurrentPCB();
     
     if (parentPCB == newPCB)
     {// set parent ID to the same processor ID for child
          // if at shell then there's no parent proccess
          newPCB->parentID = newPCB->processID; }

     else{ newPCB->parentID = parentPCB->processID; }

     // copy args over from command, trailing spaces included
     strcpy((int8_t*)newPCB->arguments, (int8_t*)(command+argIdx));

     // set up paging
     create_page_dir_entry(pid*_4MB + _8MB);
     
     // load file into memory
     read_data(dentry.inodeNum, 0, (uint8_t*) (_128MB + PROG_IMG_OFFSET), (uint32_t) _4MB);

     /* ************ Context Switch ************ */

     // prepare for context switch
     // push 5 things onto stack and IRET
     /*   ss
          esp <== what we want
          eflags
          cs
          eip
          error code */
     tss.esp0 = PCB_ADDR - PCB_8KB*(pid) - 4; // Subt 4 bytes per DOCS
     tss.ss0 = KERNEL_DS;

     //user DS is ss
     // printf("Program %d running...\n", currentProccess);
     printf("Process %d running...\n", processPerTerminal[newPCB->running_in_terminal_num][0]);


     //Page bottom -> 0x83FFFFC = 128MB + 4MB - 4
     asm volatile(
          
          "movl     %0,  %%ds;" // Put USER_DS into DS
          "pushl    %0       ;" // Push USER_DS onto kernel stack
          "pushl    %1       ;" // Push bottom of page location onto kernel stack
          "pushfl            ;" // Save flags
          "pushl    %2       ;" // Push USER_CS onto kernel stack
          "pushl    %3       ;" // Push entry point onto kernel stack
          "iret              ;"
          "IRET_OUT:         ;"
          "leave             ;"
          "ret               ;"
          :
          : "r"(USER_DS), "r"(PG_BOTTOM), "r"(USER_CS), "r"(entry_pt)
          : "eax"
     );

    return 0;
}



/* read
 * Inputs: file descriptor, buffer to read into, num of bytes to write
 * Returns: -1 if read fails
 * Function: read n bytes of data into given buffer */
int32_t read(int32_t fd, void * buf, int32_t nbytes){

     // int32_t fd - file descriptor that we want to read from
     // void * buf - holds what we read (we want to read into buf)
     // int32_t nbytes - number of bytes that we want to read

     pcb_t * current_pcb = getPcbById(currentProccess);

     // checks if fd is in bounds
     if((fd < 0) || (fd > MAX_FDESC)) {
          // printf("Read: Invalid FD%d\n", fd);
          return -1;
     }
     // checks if our buffer is valid
     if(buf == NULL) {
          // printf("Read: Invalid Buffer\n");
          return -1;
     }

     if (nbytes<=0)
     {
          // printf("Read: Invalid Size\n");
          return -1;
     }
     
     // checks if corresponding fd is in use
     if(current_pcb->fdArray[fd].inUse == 0) {
          // printf("Read: FD is busy\n");
          return -1;
     }
     return (current_pcb->fdArray[fd]).operations.readFD(fd, buf, nbytes);
}
int32_t write(int32_t fd, const void * buf, int32_t nbytes){
     pcb_t * current_pcb = getPcbById(currentProccess);
     // checks if fd is in bounds
     if((fd < 0) || (fd > MAX_FDESC)) {
          // printf("Write: Invalid FD%d\n", fd);
          return -1;
     }

     // checks if our buffer is valid
     if(buf == NULL) {
          // printf("Write: Invalid Buffer\n");
          return -1;
     }

     if (nbytes<=0)
     {
          // printf("Read: Invalid Size\n");
          return -1;
     }
     
     // checks if corresponding fd is in use
     if(current_pcb->fdArray[fd].inUse == 0) {
          // printf("Write: FD is busy\n");
          return -1;
     }
     return (current_pcb->fdArray[fd]).operations.writeFD(fd, buf, nbytes);
}

/* open
 * Inputs: filename
 * Returns: -1 if open fails
 * Function: open input filename */
int32_t open(const uint8_t * filename){
     uint32_t i;                        // iterating variable for going through the file descriptor array
     uint32_t current_file_type;        // the file type corresponding to "filename"
     directory_entry_t current_dentry;  // the directory entry corresponding to "filename"
     fileDesc_t * current_fd;                 // the current file descriptor in the array we're on
     pcb_t * current_pcb = getPcbById(currentProccess);               // the current pcb
     // printf("OPEN : %s\n", filename);
     // check if the filename is valid
     if(read_dentry_by_name(filename, &current_dentry) == -1) {
          // printf("Open: Read dentry by name Failed\n");
          return -1;
     }

     // look for first open space in file descriptor array
     // start at 2 because 0 = stdin, 1 = stdout
     for(i = 2; i <= MAX_FDESC; i++) {
          current_fd = &(current_pcb->fdArray[i]);
          // checks if the current fd index is in use
          if(current_fd->inUse == 0) {
               // sets current fd index to in-use and sets file position to its default value
               current_fd->inUse = 1;
               current_fd->filePos = 0;
               break;
          }
          // if file descriptor array is full, return error
          if(i == MAX_FDESC) {
               // printf("Open: All Fds are busy\n");
               return -1;
          }
     }

     // gets the file type of the current dentry
     current_file_type = current_dentry.fileType;

     switch(current_file_type) {
          // if filename corresponds to the RTC
          case FILE_RTC:
          {
               // opens the rtc, if failed, return -1
               if(rtcOpen(filename) != 0) {
                    // printf("Open: RTC open failed\n");
                    return -1;
               }
               // sets pcb fd values
               current_pcb->fdArray[i].inode = 0;
               current_pcb->fdArray[i].operations = rtcOps;
               break;
          }
          // if filename corresponds to a directory
          case FILE_DIR:
          {
               // opens the directory, if failed, return -1
               if(fs_open_directory(filename) != 0) {
                    // printf("Open: open Directory failed\n");
                    return -1;
               }
               // sets pcb fd values
               current_pcb->fdArray[i].inode = 0;
               current_pcb->fdArray[i].operations = fsDirOps;
               break;
          }
          // if filename corresponds to a file
          case FILE_NORMAL:
          {
               // opens the file, if failed, return -1
               if(fs_open(filename) != 0) {
                    // printf("Open: open file failed\n");
                    return -1;
               }
               // sets pcb fd values
               current_pcb->fdArray[i].inode = current_dentry.inodeNum;
               (current_pcb->fdArray[i]).operations = fsOps;
               break;
          }
     }
     // returns index of file descriptor
     return i;
}

/* close
 * Inputs: file descriptor
 * Returns: -1 if close fails
 * Function: Closes the file descriptor */
int32_t close(int32_t fd){
     pcb_t * current_pcb = getPcbById(currentProccess);

     // checks if fd is in bounds, if not, return -1
     if((fd < 2) || (fd > 7)) { // 2 to 7 FD range per MP docs
          // printf("Close: Invalid FD%d\n", fd);
          return -1;
     }

     // checks if the file descriptor corresponding to fd is in use, if not, return -1
     if(current_pcb->fdArray[fd].inUse == 0) {
          // printf("Close: FD%d already closed\n", fd);
          return -1;
     }

     // sets fd to not in-use
     current_pcb->fdArray[fd].inUse = 0;

     // attempts to close fd entry, if failed, return -1
     if(current_pcb->fdArray[fd].operations.closeFD(fd) != 0) {
          // printf("Close: Unable to Close FD%d\n", fd);
          return -1;
     }
     return 0;
}

/* getArgs
 * Inputs: user level buffer and number of bytes to copy over
 * Returns: -1 if fails
 * Function: Copy arguments into user level buffer */
int32_t getargs(uint8_t * buf, int32_t nbytes){
     // printf("hey we are inside get args\n");
     if(nbytes <= 0 || buf == NULL){
          return -1;     
     }

     // get pointer to current process
     pcb_t* currentPCB = getPcbById(currentProccess);

     int arg_length = strlen((int8_t*)currentPCB->arguments);

     // check that args are present,null terminated, and <= nbytes
     if(arg_length == 0 || arg_length > nbytes){
          return -1;
     }

     // copy args into user space
     strncpy((int8_t*) buf, (int8_t*) currentPCB->arguments, nbytes);

     return 0;
}

/* vidmap
 * Inputs: start of the screen 
 * Returns: address of where the screen is
 * Function: maps the video memory to the place where we should read*/
int32_t vidmap(uint8_t** screen_start){
     if(!screen_start || screen_start < (uint8_t**)_128MB || screen_start >  (uint8_t**)(_128MB+_4MB)){
          return -1;
     }
     
     // get current PCB and set vidmap flag
     pcb_t * current_pcb = getCurrentPCB();

     // mark process as using video memory
     current_pcb->vidMap_flag = 1;

     remap_video_memory();

     *screen_start = (uint8_t*)(_128MB+_4MB);
     return (int32_t)*screen_start;
}

/***************** EXTRA CREDIT **********************/

/* set_handler
 * Inputs: 
 * Returns: 
 * Function:  */
int32_t set_handler(int32_t signum, void* handler_address){

     return -1;
}

/* sigreturn
 * Inputs: 
 * Returns: 
 * Function: */
int32_t sigreturn(void){

     return -1;
}
