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
    pte_page_t physical_frame_number : 20;
    pte_page_t valid: 1;
    pte_page_t swapped: 1;
    
    int deleteme;
}; 

// Per process stats.
struct MM_Stats process_stats[MM_MAX_PROCESSES];


 struct MM_MapResult MM_Map(int pid, uint32_t address, int writable) {    
    //start here
    struct MM_MapResult ret = {0};
    static char message[128];

    printf("Virtual pointer = %x\n", address);
    //we must convert from a virtual pointer to a virtual page
    //and an offset by doing the two things below
    uint32_t offset = address & MM_PAGE_OFFSET_MASK;
    uint32_t virtual_frame_number = address >> MM_PAGE_SIZE_BITS;

    printf("Virtual frame number = %x\n", virtual_frame_number);
    printf("Offset = %x\n", offset);

    struct page_table_entry *page_table = (void*)phys_mem; //This is wrong according to Sheahan
    struct page_table_entry *pte = &page_table[virtual_frame_number];

    printf("Found pte: physical_frame_no: 0x%x,, valid %x\n", pte->physical_frame_number, pte->valid);

    for(int i = MM_ALL_PAGE_TABLES_SIZE_PAGES; i < MM_PHYSICAL_PAGES; i++){
        printf("Considering physical frame %d\n", i);
        if(used_pages[i] == 0){
            used_pages[i] = 1;
            pte->physical_frame_number = i;
            pte->valid = 1;
            ret.new_mapping = 1;
            ret.physical_frame = i;
            return ret;
        }
    }

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
    uint32_t offset = address & MM_PAGE_OFFSET_MASK;
    uint32_t virtual_frame_number = address >> MM_PAGE_SIZE_BITS;

    struct page_table_entry *page_table = (void*)phys_mem; //This is wrong according to Sheahan
    struct page_table_entry *pte = &page_table[virtual_frame_number];

    //need a physical_pointer
    uint32_t physical_pointer = (pte->physical_frame_number << MM_PAGE_SIZE_BITS) | offset;
    //used as an index into phys_mem

    return -1;
} 

int MM_StoreByte(int pid, uint32_t address, uint8_t value) {
	return -1;
}	

int MM_GetStats(int pid, struct MM_Stats *stats) {
	return -1;
}




