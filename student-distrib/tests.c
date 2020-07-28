#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "./devices/keyboard.h"
#include "./devices/rtc.h"
#include "paging/paging.h"
#include "fileSystem/fileSystem.h"

#define PASS 1
#define FAIL 0

#define FS_BUFFER_LARGE		100000
#define CLEAN_TERMINAL_LOOK 40
#define SIZE_OF_FS_STR		5
/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	 asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 20 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) && (idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

/* divideByZero;
 * Inputs: void
 * Return Value: FAIL(only when test fails) => technically it freezes before returning anything
 * Function: Trigger a zero exception
 * Coverage: IDT exceptions
 * Files: interrupts-exceptions/idt.c, interrupts-exceptions/idt.h,
 * 		  interrupts-exceptions/exceptionHandlers.c, interrupts-exceptions/exceptionHandlers.h
 * 		  linkages/exception_wrapper.h, linkages/exception_wrapper.S 
 */
int divideByZero(){
	int zero = 0;
	zero = 1/zero;
	
	return FAIL;
}


/* idt_test_segmentFault;
 * Inputs: void
 * Return Value: PASS or FAIL
 * Function: Dereferences address zero to trigger exception 
 * Coverage: IDT exceptions (seg fault)
 * Files: interrupts-exceptions/idt.c, interrupts-exceptions/idt.h,
 * 		  interrupts-exceptions/exceptionHandlers.c, interrupts-exceptions/exceptionHandlers.h
 * 		  linkages/exception_wrapper.h, linkages/exception_wrapper.S 
 */
int idt_test_segmentFault(){
	TEST_HEADER;
	
	int * test;
	test = 0; // This is causing page fault, should it be seg fault??
	int x = *test;
	x = x + 2;
	
	assertion_failure();
	return FAIL;
}

/* checkpoint1_tests;
 * Inputs: The number of the exception you want to raise
 * Return Value: PASS or FAIL
 * Function: Forces an interrupt at the desired exception to test if exceptions are working 
 * Coverage: All of IDT
 * Files: interrupts-exceptions/idt.c, interrupts-exceptions/idt.h,
 * 		  interrupts-exceptions/exceptionHandlers.c, interrupts-exceptions/exceptionHandlers.h
 * 		  linkages/exception_wrapper.h, linkages/exception_wrapper.S 
 */
void checkpoint1_tests(int testKey){

	switch (testKey)
	{
	case '0': // Test divideError
		divideByZero();
		break;

	case '1': // Test reserved
		asm volatile("int $1"); 
		break;
	
	case '2': // Test NMINT 
		asm volatile("int $2"); 
		break;

	case '3': // Test breakpoint
		asm volatile("int $3"); 
		break;

	case '4': // Test overflow
		asm volatile("int $4"); 
		break;

	case '5': // Test boundRangeExceeded
		asm volatile("int $5"); 
		break;

	case '6': // Test invalidOpcode
		asm volatile("int $6"); 
		break;

	case '7': // Test deviceNA
		asm volatile("int $7"); 
		break;

	case '8': // Test doubleFault
		asm volatile("int $8"); 
		break;

	case '9': // Test segmentOverrrun
		asm volatile("int $9"); 
		break;
	case 's': // System call vector
		asm volatile("int $128");
		break;

	case 10: // Test invalidTSS
		asm volatile("int $10"); 
		break;

	case 11: // Test segmentNotPresent
		asm volatile("int $11"); 
		break;

	case 12: // Test segmentFault -> called stack fault in docs???
		asm volatile("int $12"); 
		break;

	case 13: // Test generalProtection
		asm volatile("int $13"); 
		break;

	case 14: // Test pageFault
		asm volatile("int $14"); 
		break;

	case 16: // Test mathFault
		asm volatile("int $16"); 
		break;

	case 17: // Test alignmentCheck
		asm volatile("int $17"); 
		break;

	case 18: // Test machineCheck
		asm volatile("int $18"); 
		break;

	case 19: // Test floatingPointException
		asm volatile("int $19"); 
		break;
	}
}

// paging tests

/* Page Table/Page Directory Exists
 * 
 * Asserts whether or not the page table/page directory have been initialized correctly
 * Inputs: None
 * Outputs: PASS/FAIL, if FAIL, which part failed
 * Side Effects: None
 * Coverage: page table/ page directory initialized properly
 * Files: paging/paging.c, paging/paging.h
 */
int paging_init_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for(i = 0; i < 1024; i++){
		// tests if video memory has been correctly added to the page table
		if(i == 0xB8) {
			// tests present (should be 1)
			if((page_table[i] & 0x00000001) == 0) {
				printf("Video Memory: Present not set \n");
				result = FAIL;
			}
			// tests R/W (should be 1)
			if((page_table[i] & 0x00000002) == 0) {
				printf("Video Memory: R/W not set \n");
				result = FAIL;
			}
		}
		// tests if each PTE has been correctly set to R/W
		else {
			if((page_table[i] & 1) == 1) {
				printf("Generic PTE: Present set \n");
				result = FAIL;
			}
			if((page_table[i] & 0x00000002) == 0) {
				printf("Generic PTE: R/W not set \n");
				result = FAIL;
			}
		}

		// tests if page table has been added to the page directory
		if(i == 0) {
			if((page_directory[i] & 0x00000001) != 1) {
				printf("Page Table in Page Directory: Present not set \n");
				result = FAIL;
			}
			// tests R/W (should be 1)
			if((page_directory[i] & 0x00000002) != 2) {
				printf("Page Table in Page Directory: R/W not set \n");
				result = FAIL;
			}
		}
		// tests if kernel memory has been correctly set
		else if(i == 1) {
			// tests Present (should be 1)
			if((page_directory[i] & 0x00000001) != 1) {
				printf("Kernel Memory: Present not set \n");
				result = FAIL;
			}
			// tests R/W (should be 1)
			if((page_directory[i] & 0x00000002) != 2) {
				printf("Kernel Memory: R/W not set \n");
				result = FAIL;
			}
			// tests Page Size (should be 1)
			if((page_directory[i] & 0x00000080) != 128) {
				printf("Kernel Memory: Page Size not set \n");
				result = FAIL;
			}
		}
		// tests if other PDEs have been correctly set to R/W
		else {
			if((page_directory[i] & 0x00000002) != 2) {
				printf("Generic PDE: R/W not set \n");
				result = FAIL;
			}
		}
	}
	return result;
}

/* Dereferencing video/kernel memory
 * 
 * Asserts the video/kernel memory pages exist
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: If video/kernel memory pages don't exist, page fault
 * Coverage: Video/Kernel memory set up properly
 * Files: paging/paging.c, paging/paging.h
 */
int paging_dereference_existing_test() {
	TEST_HEADER;

	int * pointer;
	int value;

	// dereferencing video memory
	printf("Dereferencing Video Memory ... \n");
	pointer = (int *)(0xB8000);
	value = *pointer;

	// dereferencing kernel memory
	printf("Dereferencing Kernel Memory ... \n");
	pointer = (int *)(0x400000);
	value = *pointer;

	return PASS;
}

/* Dereferencing null
 * 
 * Asserts that attempting to dereferencing NULL results in a page fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Page fault, if successful
 * Coverage: Checks if dereferencing non-existent locations results in a page fault
 * Files: paging/paging.c, paging/paging.h
 */
int paging_dereference_null_test() {
	TEST_HEADER;

	int * pointer;
	int value;

	pointer = NULL;
	value = *pointer;

	return FAIL;
}

/* Dereferencing non-existing page
 * 
 * Asserts that attempting to dereferencing a non-existent results in a page fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Page fault, if successful
 * Coverage: Checks if dereferencing non-existent locations results in a page fault
 * Files: paging/paging.c, paging/paging.h
 */
int paging_dereference_nonexist_test() {
	TEST_HEADER;

	int * pointer;
	int value;

	pointer = (int *)(0x800001);
	value = *pointer;

	return FAIL;
}
/************************** Checkpoint 2 tests **************************************/

/* frequency_tests;
 * Inputs: The number of the exception you want to raise
 * Return Value: PASS or FAIL
 * Function: Forces an interrupt at the desired exception to test if exceptions are working 
 * Coverage: All of IDT
 * Files: interrupts-exceptions/idt.c, interrupts-exceptions/idt.h,
 * 		  interrupts-exceptions/exceptionHandlers.c, interrupts-exceptions/exceptionHandlers.h
 * 		  linkages/exception_wrapper.h, linkages/exception_wrapper.S 
 */
void frequency_tests(int new_freq){
	
	if (new_freq>0 && new_freq<10){ // 10  is max frequency for the test -> can be more in rtc code
		int32_t fd;
		int32_t* new_frequency_ptr;
		clear();
		new_frequency_ptr = &new_freq;
		if(rtcWrite(fd, new_frequency_ptr, 4) == -1){ // only write 4 bytes
			printf("Invalid Frequency\n");
		}
		testKey =0;
	}
}

/* rtc_test;
 * Inputs: void
 * Return Value: FAIL(only when test fails) => technically call never returns back
 * 				 to returning a PASS
 * Function: Trigger rtc "test interrupts"
 * Coverage: Load RTC
 * Files: devices/rtc.c, devices/rtc.h
 * 		  interrupts-exceptions/interruptHandlers.c, interrupts-exceptions/interruptHandlers.h,
 * 		  linkages/isr_wrapper.h, linnkages/isr_wrapper.S 
 */
int rtc_test(){
	// TEST_HEADER;
	clear();
	printf("Enter start: Start test(rtcOpen), close: RTC close, and # to change frequency\n");

	uint8_t* closeStr = (uint8_t*)"close";
	uint8_t* startStr = (uint8_t*)"start";
	uint8_t* filename = (uint8_t*)0x45; // dummy address for testing
	while (1)
	{	
		clear_buffer(CLR_KEYBOARD_BUFFER);
		clear_buffer(CLR_TERMINAL_BUFFER);
		while(testKey != '\n'){}

		int32_t new_freq = terminal_buffer[0] - '0';
		
		if(!strncmp((int8_t*)startStr, terminal_buffer, SIZE_OF_FS_STR)){

				rtc_test_Flag = 1;
				// change freq to 2Hz
				rtcOpen(filename);

		}
		else if(!strncmp((int8_t*)closeStr, terminal_buffer, SIZE_OF_FS_STR)){
			rtc_test_Flag =0;
			printf("Return value of rtcClose:%d\n", rtcClose(dummy_fd));
			break;
		}
		else if (new_freq>0 && new_freq<10) // 10  is max frequency for the test -> can be more in rtc code
		{ frequency_tests(new_freq);}
		
		testKey =0;
		
	}
	
	return PASS;
}



void terminal_driver_test(){
	
	uint8_t* filename = (uint8_t*)0x45; // dummy variable for testing
	uint8_t*exitStr = (uint8_t*)"exit";

	clear();
	printf("Calling terminalOpen, return value:%d\n", terminalOpen(filename));
	printf("Running Terminal Driver Test\n");
	clear_buffer(CLR_KEYBOARD_BUFFER);
	while(1){
		clear_buffer(CLR_KEYBOARD_BUFFER);
		clear_buffer(CLR_TERMINAL_BUFFER);
		while(testKey != '\n'){}
		// enter is pressed
		terminalWrite(dummy_fd, terminal_buffer, buffer_read_bytes);
		testKey =0;

		if(!strncmp((int8_t*)exitStr, terminal_buffer, 4)){ // size of exit string
			printf("Calling terminalClose, return value:%d\n", terminalClose(dummy_fd));
			return;
		}
	}
}
/* file_system_list_test;
 * Inputs: N/A
 * Return Value: Void
 * Function: Tests fs 
 * Coverage: All file system
 * Files: fileSystem/fileSystem.h
 */
void file_system_list_test(){
	// TEST_HEADER;
	// clear();
	// uint8_t* closeStr = (uint8_t*)"close";
	// uint8_t* writeStr = (uint8_t*)"write";
	// uint8_t buffer[FS_BUFFER_LARGE];
	// directory_entry_t file;
	// int32_t read;
	// int32_t i;
	// int32_t j;
	// while(1){
	// 	clear_buffer(CLR_KEYBOARD_BUFFER);
	// 	clear_buffer(CLR_TERMINAL_BUFFER);

	// 	printf("Directory: \n");
	// 	for(i = 0; i < DENTRY_MAX_SIZE; i++){
	// 		if(read_dentry_by_index((uint32_t)i, &file) == -1) break;
	// 		printf("%s",file.fileName);
	// 		read = read_data(file.inodeNum, 0, buffer, FS_BUFFER_LARGE);
	// 		for (j = strlen((int8_t*)file.fileName); j < CLEAN_TERMINAL_LOOK; j++){
	// 			printf(" ");
	// 		}
	// 		printf("Size(B): %d\n", read);
	// 	}
	// 	printf("Enter file name to read, 'close' to close, 'write' to write: \n");
	// 	while(testKey != '\n'){}
	// 	if(!strncmp((int8_t*)closeStr, terminal_buffer, SIZE_OF_FS_STR)){
	// 		clear();
	// 		// printf("Return value for fs_close(): %d \n", fs_close());
	// 		printf("FS closed \n");
	// 		return;
	// 	}
	// 	if(!strncmp((int8_t*)writeStr, terminal_buffer, SIZE_OF_FS_STR)){
	// 		clear();
	// 		printf("Return value for fs_write(): %d \n", fs_write(dummy_fd,buffer, FS_BUFFER_LARGE));
	// 		// testKey = 0;
	// 	}
	// 	else{
	// 		clear();
	// 		read = fs_read((uint8_t*)terminal_buffer, buffer, FS_BUFFER_LARGE);
	// 		if(read != -1){
	// 			for(i = 0; i < read; i++){
	// 				if(buffer[i] != 0x00)
	// 					printf("%c", buffer[i]);
	// 			}
	// 		}
	// 		else{
	// 			printf("FAIL\n");
	// 		} 
	// 	}
	// 	testKey = 0;
	// 	printf("\nPress c to continue testing \n");
	// 	while(testKey!= 'c'){}
	// 	clear();
	// }
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	printf("Press the coressponding key to run tests(ZERO to turn off Test Mode):\n");
	printf("R: RTC\nT: terminal\nF: to pay respects and run file system \n");
	// launch your tests here
	int testMODE =1;
	while (testMODE){
		switch (testKey)
		{
			case 'r':
				TEST_OUTPUT("RTC Open called, Frequency = 2Hz", rtc_test());
				testKey =0;
				break;

			case '0':
				testMODE = 0; break;

			case 't':
				terminal_driver_test();
				testKey = 0;
				break;

			case 'f':
				file_system_list_test();
				testKey = 0;
				break;
		}
	}
	printf("\n No test has been initiated, FreeRoam mode ;)\n");

	// checkpoint1_tests(11); // change number in exception test to trigger others
	//TEST_OUTPUT("idt_test_segmentFault", idt_test_segmentFault());
	
}
