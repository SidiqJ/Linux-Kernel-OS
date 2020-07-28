#ifndef FILES_H
#define FILES_H

#include "../types.h"
#include "../x86_desc.h"
#include "../lib.h"
#include "../systemCalls/pcb.h"


#define DENTRY_SIZE         64
#define NAME_MAX_SIZE       32
#define DENTRY_MAX_SIZE     63

#define DATA_BLOCK_SIZE     4096
#define INODE_MAX_BLOCK     1023

#define FS_BUFFER_LARGE		100000

#define FAIL_FS             -1
#define SUCCESS_FS          0

#define FILE_RTC            0
#define FILE_DIR            1
#define FILE_NORMAL         2

#define MAX_SIZE_FNAME      32

#define DENTRY_RESERVED     24
#define BB_RESERVED         52


/* struct of a dentry */
typedef struct {
    uint8_t    fileName[NAME_MAX_SIZE];
    uint32_t   fileType;
    uint32_t   inodeNum;
    uint8_t    reserved[DENTRY_RESERVED];
} directory_entry_t;

/* struct of an inode */
typedef struct {
    uint32_t    length;
    uint32_t    dataBlocks[INODE_MAX_BLOCK];
} inode_t;

/* struct of a boot block */
typedef struct {
    uint32_t    dentry;
    uint32_t    inode;
    uint32_t    dataBlocks;
    uint8_t     reserved[BB_RESERVED];
    directory_entry_t dentry_array[DENTRY_MAX_SIZE];
} boot_block_t;

inode_t* inode_array;
/* ptr to where data starts */
uint32_t data_begin_ptr;


int32_t fs_open(const uint8_t* filename);

void fs_init(uint32_t addr);

int32_t fs_close(int32_t fd);

int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes);

//int32_t fs_read(uint8_t* fileName, uint32_t offset, uint8_t* buffer, uint32_t length);
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes);

int32_t read_dentry_by_name(const uint8_t* fileName, directory_entry_t* dentry);

int32_t read_dentry_by_index(uint32_t index, directory_entry_t* dentry);

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buffer, uint32_t length);

int32_t fs_read_directory(int32_t fd, void* buf, int32_t nbytes);

int32_t fs_write_directory(int32_t fd, const void* buf, int32_t nbytes);

int32_t fs_open_directory(const uint8_t* filename);

int32_t fs_close_directory(int32_t fd);



#endif
