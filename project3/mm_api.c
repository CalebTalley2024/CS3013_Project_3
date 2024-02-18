#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mm_api.h"

///////////////////////////////////////////////////////////////////////////////
// All implementation goes in this file.                                     //
///////////////////////////////////////////////////////////////////////////////

uint8_t phys_mem[MM_PHYSICAL_MEMORY_SIZE_BYTES];

// Section 2: File for swap, if backed by disk.
FILE *swap_disk_file;

// A single page table entry.
struct page_table_entry {
	// pte_page_t page; // What goes here??
	int deleteme;
};

// Per process stats.
struct MM_Stats process_stats[MM_MAX_PROCESSES];

struct MM_MapResult MM_Map(int pid, uint32_t address, int writable) {	
	struct MM_MapResult ret = {0};
	static char message[128];
	sprintf(message, "unimplemented");
	ret.message = message;
	ret.error = 1;
	return ret;
}

void MM_AutoMap() {
	// ?
}

void MM_SwapOn() {
	// ?
}

uint32_t MM_PageSize() { return MM_PAGE_SIZE_BYTES; }

int MM_LoadByte(int pid, uint32_t address, uint8_t *value) {
	return -1;
}

int MM_StoreByte(int pid, uint32_t address, uint8_t value) {
	return -1;
}	

int MM_GetStats(int pid, struct MM_Stats *stats) {
	return -1;
}

