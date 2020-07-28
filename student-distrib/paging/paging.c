#include "paging.h"

/* 
 * init_paging
 *   DESCRIPTION: Initializes the page table and page directory,
 *                enables paging through the control registers, 
 *                and initializes pages for kernel and video memory
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: edits the control registers, clobbers eax
 */
void init_paging() {
    // scheduling_backup[0] = 0xB9000;
    // scheduling_backup[1] = 0xBA000;
    // scheduling_backup[2] = 0xBB000;
    uint32_t i;     // variable used for iterating through page_directory
    uint32_t j;     // variable used for iterating through page_table

    // creates a blank page directory, with each entry having the R/W bit set
    for(i = 0; i < NUM_ENTRIES; i++) {
        page_directory[i] = RW_ENABLE;
    }

    // initializes page table, each with the address and the R/W bit set
    for(j = 0; j < NUM_ENTRIES; j++) {
        page_table[j] = j << ADDR_BIT_SHIFT;
        page_table[j] |= RW_ENABLE;
    }

    // puts page table in page directory with R/W and Present flags set
    // first 4 MB, should be made up of 4 kB pages
    page_directory[0] = (uint32_t)page_table;
    page_directory[0] |= RW_PRESENT_ENABLE;

    // 4-8 MB (kernel, should be a single 4 mB page)
    // in Appendix C, kernel loaded at physical address 0x400000
    // sets Page Size, R/W, and Present bits
    page_directory[1] = KERNEL_MEMORY_ADDR;
    page_directory[1] |= RW_PRESENT_SIZE_ENABLE;
    
    // video memory located at 0xB8000, from lib.c
    // sets R/W and Present flags
    page_table[VIDEO_MEM_ADDR] |= RW_PRESENT_ENABLE;

    // terminal one backup
    page_table[0xB9] = 0xB9000 | RW_PRESENT_ENABLE;
    // terminal two backup
    page_table[0xBA] = 0xBA000 | RW_PRESENT_ENABLE;
    // terminal three backup
    page_table[0xBB] = 0xBB000 | RW_PRESENT_ENABLE;

    flushTLB();


    remap_video_memory();
    // enable paging in x86
    asm volatile (
        "/* CR3 stores the physical address of the first page directory entry */"
        "movl $page_directory, %%eax;"
        "movl %%eax, %%cr3;"
        "/* need to enable PSE to use 4 mB pages, which is bit 4 of CR4 */"
        "/* hence 0x00000010 -> cr4 */"
        "movl %%cr4, %%eax;"
        "orl $0x00000010, %%eax;"
        "movl %%eax, %%cr4;"
        "/* 32th bit of CR0 enables paging */"
        "/* 1st bit of CR0 sets the protection bit */"
        "/* -> cr0 = 0x80000000 */"
        "movl %%cr0, %%eax;"
        "orl $0x80000000, %%eax;"
        "movl %%eax, %%cr0;"
        : /* no outputs */
        : /* no inputs */ 
        : "eax" /* eax is clobbered */
    );
}

/* create_page_dir_entry
 * Inputs: physical memory address
 * Outputs: none
 * Function: maps page directory at VA 128MB to desired physical address */
void create_page_dir_entry(uint32_t p_addr){

    // change PDE for different process
    page_directory[VA_128MB] = p_addr | PE_MASK; // mask with correct bits 
    
    // flush TLB for new process
    flushTLB();
}
/* remap_video_memory
 * Inputs: void
 * Outputs: void
 * Function: enables the video page table and the directory where vid page is stored */
void remap_video_memory(void){
    //page_directory[34] = user_page_table[0] | RW_PRESENT_ENABLE_USER;
    page_directory[VID_PAGE_INDEX] = (uint32_t)video_table | RW_PRESENT_ENABLE_USER;
    video_table[0] = VIDEO_MEM_PHY_ADDR | RW_PRESENT_ENABLE_USER;
    flushTLB();
}


/* reclaim_video_memory
 * Inputs: void
 * Outputs: void
 * Function: disables the video page table and the directory where vid page is stored */
void reclaim_video_memory(void){
    //page_directory[34] = user_page_table[0] | RW_PRESENT_ENABLE_USER;
    page_directory[VID_PAGE_INDEX] = (uint32_t)video_table | RW_PRESENT_DISABLE_USER;
    video_table[0] = VIDEO_MEM_PHY_ADDR | RW_PRESENT_DISABLE_USER;
    flushTLB();
}
/* flushTLB
 * Inputs: none
 * Outputs: none
 * Function: flushes the TLB (CR3) */
void flushTLB(){
    asm volatile("   "
    "movl %%cr3, %%eax;"
    "movl %%eax, %%cr3;"
    : /* no outputs */
    : /* no inputs */ 
    : "eax" /* eax is clobbered */
    );
}
