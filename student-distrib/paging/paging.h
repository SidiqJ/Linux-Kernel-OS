#ifndef PAGING_H__
#define PAGING_H__

#include "../types.h"

/* From https://wiki.osdev.org/Paging
 * "Paging is a system which allows each process to see a full virtual address space,
 * without actually requiring the full amount of physical memory to be 
 * available or present."
 */

#define NUM_ENTRIES             1024
#define RW_ENABLE               2
#define RW_PRESENT_ENABLE       3
#define RW_PRESENT_ENABLE_USER  7
#define RW_PRESENT_DISABLE_USER  0x6

#define RW_PRESENT_SIZE_ENABLE  131
#define RW_PRESENT_USER_SIZE_ENABLE 135
#define ADDR_BIT_SHIFT          12
// physical address of the kernel (4 mB), Appendix C
#define KERNEL_MEMORY_ADDR      0x400000
// from lib.c, 0xB8000 >> 12
#define VIDEO_MEM_ADDR          0xB8
#define VIDEO_MEM_PHY_ADDR      0xB8000
#define PD_PT_ALIGN             4096
#define VA_128MB                32
#define VID_PAGE_INDEX          33
#define PE_MASK                 0x87

#define VIDEO_MEM_ADDR_1        0xB9
#define VIDEO_MEM_ADDR_2        0xBA
#define VIDEO_MEM_ADDR_3        0xBB
#define _128MB                  0x08000000
#define _4MB                    0x0400000
#define _8MB                    0x0800000



// table in memory which the MMU uses to find the page tables
// bit 0: Present
// bit 1: Read/Write permissions flag
// bit 2: User/Supervisor
// bit 3: Write-Through
// bit 4: Cache Disable
// bit 5: Accessed
// bit 6: 0
// bit 7: Page Size (0 for 4kb)
// bit 8: Ignored
// bit 9-11: Available
// bit 12-31: Page Table 4-kb aligned address
uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(PD_PT_ALIGN))); // contains 1024 4-byte entries

// table in memory that describes how the MMU should translate a certain range of addresses
// bit 0: Present
// bit 1: Read/Write
// bit 2: User/Supervisor
// bit 3: Write Through
// bit 4: Cache Disabled
// bit 5: Accessed
// bit 6: Dirty
// bit 7: 0
// bit 8: Global
// bit 9-11: Available
// bit 12-31: Physical Page Address
uint32_t page_table[NUM_ENTRIES] __attribute((aligned(PD_PT_ALIGN)));

uint32_t video_table[NUM_ENTRIES] __attribute__((aligned(PD_PT_ALIGN)));

// uint32_t scheduling_backup[3];
// initializes the page table/page directory, enables paging, and initializes pages for kernel/video memory
extern void init_paging();

extern void flushTLB();

extern void create_page_dir_entry(uint32_t p_addr);

void remap_video_memory(void);

void reclaim_video_memory(void);

#endif /* PAGING_H__*/
