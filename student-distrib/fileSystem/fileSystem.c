#include "fileSystem.h"

/* checks if file is already open*/
uint8_t file_open_bool = 0x0;
/* holds all the dentries initialized on boot */
directory_entry_t* boot_dentries;
/*points to where the boot_block is */
uint32_t boot_block_ptr;
/* holds everything stored in boot_block */
boot_block_t boot_block;
/* array of inodes */
// inode_t* inode_array;
// /* ptr to where data starts */
// uint32_t data_begin_ptr;
/*counts directory reads for dir_read */
uint32_t directory_read_count = 0;

/* fs_init
 * Description: initializes file system and sets globals
 * Inputs: starting point
 * Returns: returns -1 on fail 0 on success */
void fs_init(uint32_t addr){
    if(file_open_bool) return;
    boot_block_ptr = addr;
    memcpy(&boot_block, (void*)boot_block_ptr, DENTRY_SIZE);
    data_begin_ptr = boot_block_ptr + (boot_block.inode+1)*DATA_BLOCK_SIZE;
    boot_dentries = (directory_entry_t*)(boot_block_ptr + DENTRY_SIZE);
    inode_array = (inode_t*) (boot_block_ptr + DATA_BLOCK_SIZE);
    file_open_bool = 1;
}
/* fs_open
 * Description: opens file
 * Inputs: starting point
 * Returns: returns -1 on fail 0 on success */

int32_t fs_open(const uint8_t* filename){
    // if(file_open_bool) return FAIL_FS;
    // fs_init(addr);
    file_open_bool = 1;
    return SUCCESS_FS;
    // return 0;
}

/* fs_close
 * Description: closes file system and allows reinit
 * Inputs: none
 * Returns: returns -1 on fail 0 on success */

int32_t fs_close(int32_t fd){
    // if(!file_open_bool) return FAIL_FS;
    file_open_bool = 0;
    return SUCCESS_FS;
}
/* fs_write
 * Description: does nothing
 * Inputs: none
 * Returns: returns -1 on fail 0 on success */
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes){ return FAIL_FS;}

/* fs_read
 * Description: reads based on file name and offset
 * Inputs: file name, byte offset, buffer to write to, how much to write
 * Returns: returns -1 on fail, bytes read on success */
/*
int32_t fs_read(uint8_t* fileName, uint32_t offset, uint8_t* buffer, uint32_t length){
    directory_entry_t directory_entry;
    if(!fileName || !buffer) return FAIL_FS;
    if(sizeof(fileName)>= MAX_SIZE_FNAME) return FAIL_FS;
    if(FAIL_FS == read_dentry_by_name(fileName, &directory_entry)) return FAIL_FS;
    return read_data(directory_entry.inodeNum, offset, buffer, length);
}*/
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes){
    // directory_entry_t directory_entry;
    uint32_t inode;
    uint32_t offset;
    int32_t read;
    pcb_t* pcb;
    pcb = getCurrentPCB();
    if(!buf || !pcb) return FAIL_FS;
    inode = pcb->fdArray[fd].inode;
    offset = pcb->fdArray[fd].filePos;
    read = read_data(inode, offset, (uint8_t*)buf, (uint32_t)nbytes);
    if(read == FAIL_FS) return FAIL_FS;
    pcb->fdArray[fd].filePos += read;
    return read;
    
}

/* read_dentry_by_name
 * Description: reads based on file name
 * Inputs: file name, dentry pointer
 * Returns: returns -1 on fail, 0 on success */

int32_t read_dentry_by_name(const uint8_t* fileName, directory_entry_t* dentry){
    uint32_t i, fnLength;
    for ( fnLength = 0; fnLength <= MAX_SIZE_FNAME; fnLength++)
    {   if (fileName[fnLength]== ' ' || fileName[fnLength] == '\0')
        { break;}        
    }

    //// We added this for testing
    // printf("Opening file: %s\n", fileName);
    // printf("file Length : %d\n", fnLength);
    
    if(!fileName || !dentry || !fnLength || fnLength>MAX_SIZE_FNAME){ 
        return FAIL_FS;
    }

    for(i = 0; i < DENTRY_MAX_SIZE; i++){
        /* copies metadata from boot_dentries to dentry if length and name are equal */
        uint32_t currentFileNameLength = strlen((int8_t*)boot_dentries[i].fileName);
        if(currentFileNameLength > MAX_SIZE_FNAME) currentFileNameLength = MAX_SIZE_FNAME;
        if(currentFileNameLength == fnLength &&
          !strncmp((int8_t*)boot_dentries[i].fileName, (int8_t*)fileName, fnLength-1))
        {
            memcpy(dentry, &boot_dentries[i], DENTRY_SIZE);
            if(sizeof(boot_dentries[i].fileName) >= MAX_SIZE_FNAME){
                dentry->fileName[MAX_SIZE_FNAME] = '\0'+dentry->fileName[MAX_SIZE_FNAME];
            }
            return SUCCESS_FS;
        }
    }
    return FAIL_FS;
}

/* read_dentry_by_index
 * Description: reads based on inode index
 * Inputs: index, dentry pointer
 * Returns: returns -1 on fail, 0 on success */
int32_t read_dentry_by_index(uint32_t index, directory_entry_t* dentry){
    if(index >= DENTRY_MAX_SIZE 
        || !dentry
        || (boot_dentries[index].fileType != FILE_NORMAL
        && boot_dentries[index].fileType != FILE_RTC
        && boot_dentries[index].fileType != FILE_DIR)
        || index >= boot_block.dentry) 
            return FAIL_FS;
    memcpy(dentry, &boot_dentries[index], DENTRY_SIZE);
    if(sizeof(boot_dentries[index].fileName) >= MAX_SIZE_FNAME){
        dentry->fileName[MAX_SIZE_FNAME] = '\0' + dentry->fileName[MAX_SIZE_FNAME];
    }
    return SUCCESS_FS;
}
/* read_data
 * Description: reads data based on inode
 * Inputs: inode, byte offset to read from, buffer to write to, how much to write
 * Returns: returns -1 on fail, bytes read on success */

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buffer, uint32_t length){
    uint32_t read_success =         0;
    uint32_t block_location_ptr =   offset % DATA_BLOCK_SIZE; /* this is the specific point in the data block we're reading */
    uint32_t data_block =           offset/DATA_BLOCK_SIZE; /* this is the specific data block we read from */
    uint8_t* read_buffer_ptr;
    /*if inode is larger than total we fail or datablock is beyond boot block data block or if buffer is invalid*/
    if(inode >= boot_block.inode 
        || inode_array[inode].dataBlocks[data_block] >= boot_block.dataBlocks 
        || !buffer) 
            return FAIL_FS;
    /* if our offset is larger than whats in this inode we finished reading from possible inodes */
    if(offset >= inode_array[inode].length || length == 0) return SUCCESS_FS;

    /* we can point to where we're going to read from based on this offset */    
    read_buffer_ptr = (uint8_t*)(data_begin_ptr +
                           inode_array[inode].dataBlocks[data_block]*DATA_BLOCK_SIZE + 
                           block_location_ptr);

    /* loop through everything we're trying to read */
    for(read_success = 0; read_success < length; read_success++){
        /* Checks EOF  THIS SHOULD REALLY CHECK IF IT PASSES MAX FILE SIZE RATHER THAN JUST FILE LEN*/
        if(read_success + offset >= inode_array[inode].length) break;
        /* if our block point is = or greater than size of the block we move to next block */
        if(block_location_ptr >= DATA_BLOCK_SIZE){
            block_location_ptr = 0;
            data_block++;
            if(inode_array[inode].dataBlocks[data_block] >= boot_block.dataBlocks) return FAIL_FS;
            read_buffer_ptr = (uint8_t*)(data_begin_ptr + inode_array[inode].dataBlocks[data_block]*DATA_BLOCK_SIZE);
        }
        /* we just inidvidually move the byte back in */
        buffer[read_success] = *read_buffer_ptr;
        block_location_ptr++;
        read_buffer_ptr++;
    }
    return read_success;
}
/* fs_read_directory
 * Description: sets given buffer to next file name in the inode
 * Inputs: buffer
 * Returns: returns -1 on fail, size of file on success */

int32_t fs_read_directory(int32_t fd, void* buf, int32_t nbytes){
    if(!buf) return 0;
    directory_entry_t dentry;
    //uint8_t dataSizeBuffer[FS_BUFFER_LARGE];
    if(read_dentry_by_index(directory_read_count, &dentry) == -1){
        directory_read_count = 0;
        return 0;
    }
    directory_read_count++;
    strncpy((int8_t*)buf, (int8_t*)dentry.fileName, sizeof(dentry.fileName));
    return strlen(buf);
}
/* fs_write_directory
 * Description: does nothing 
 * Inputs: void
 * Returns: fail */
int32_t fs_write_directory(int32_t fd, const void* buf, int32_t nbytes){return FAIL_FS;}
/* fs_open_directory
 * Description: does nothing 
 * Inputs: void
 * Returns: success */
int32_t fs_open_directory(const uint8_t* filename){return SUCCESS_FS;}
/* fs_close_directory
 * Description: does nothing 
 * Inputs: void
 * Returns: success */
int32_t fs_close_directory(int32_t fd){return SUCCESS_FS;}
