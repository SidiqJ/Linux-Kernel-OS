#ifndef SYSTEMCALLS_H__
#define SYSTEMCALLS_H__

#include "../types.h"
#include "../paging/paging.h"
#include "../lib.h"
#include "../fileSystem/fileSystem.h"
#include "../x86_desc.h"
#include "../devices/rtc.h"

#define _4MB        0x0400000
#define _8MB        0x0800000
#define _128MB      0x08000000
#define PROG_IMG_VA 0x08048000
#define PROG_IMG_OFFSET 0x00048000
#define MAX_PROCESSES 6
#define PG_BOTTOM 0x83FFFFC
#define KEYBOARD_BUFFER_SIZE 128
#define EXCEPTION_STATUS 256
#define COMMAND_BUFF_SIZE 4
#define BUFF_READ_START 24
#define MAX_FDESC 7

uint32_t entry_pt;




int32_t halt(uint8_t status);
int32_t execute(const uint8_t * command);
int32_t read(int32_t fd, void * buf, int32_t nbytes);
int32_t write(int32_t fd, const void * buf, int32_t nbytes);
int32_t open(const uint8_t * filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t * buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

#endif
