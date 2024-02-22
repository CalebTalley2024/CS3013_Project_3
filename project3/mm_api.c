#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mm_api.h"

///////////////////////////////////////////////////////////////////////////////
// All implementation goes in this file.                                     //
///////////////////////////////////////////////////////////////////////////////

// uint8_t has smaller range than 'int', only takes up 1 byte vs 4 bytes
// range of 8 bit (1 byte): 0 - 255: 11111111 in binary = 255
// first 4 page frames are reserved for process page tables
uint8_t phys_mem[MM_PHYSICAL_MEMORY_SIZE_BYTES];

// A single page table entry.
// pte_page_t page; // What goes here??
 struct Page_Table_Entry {
    pte_page_t physical_frame_number : 20; 
    pte_page_t valid: 1;
    pte_page_t swapped: 1;
    pte_page_t writable: 1;

};


//Chris's initial idea (not the real one)
//We have an array of page tables. Maybe we can put this array of
//page tables inside physical memory. In this way, we can access
//these page tables by pointing to this part of phys_mem, and then
//accessing the pid part of the array by using the pid #. page_tables[pid]
//start of phys_mem -> page_tables[pid]


// create 4 page table pointers
// struct Page_Table *page_tables[MM_MAX_PROCESSES];


// init page table pointers

//////////////////////// By Google Instructor

// struct page_table_entry *pte = &start_of_page_table[virtual_frame_number];

// struct page_table_entry* page_table_start = &phys_mem[pid * MM_PAGE_TABLE_SIZE_BYTES];


/////////////////////////

// page table location reister: holds location of the start of each processes'page table
uint8_t *page_table_loc_register[MM_MAX_PROCESSES];

int create_page_table_ptr(int pid){
    // make register for pid point to base + offset
    // base = location of the first page in phy_mem
    // offset  = size of a page * pid               (location where the page table should be)
    // We are assuming that each Page table is the same size, that way we can use pid to move the pointer by an offset each time
    int offset = MM_PAGE_TABLE_SIZE_BYTES* pid;
    // printf("\nhello");
    page_table_loc_register[pid] =  (uint8_t*)(&phys_mem + offset);
    /*

    (uint8_t*): cast address from sum
    
    (&phys_mem + offset): sum for differnt memory address
    
    */

    return 0;
}





uint8_t used_pages[MM_PHYSICAL_PAGES];  // default value is 0


// Per process stats.
struct MM_Stats process_stats[MM_MAX_PROCESSES];




// init_used_pages();
// init_used_pages (used_pages);
//We are here, we are here, we are here!


// #TODO use pid when getting page table
struct MM_MapResult MM_Map(int pid, uint32_t address, int writable) {  

    // make page table pointer for this pid
    create_page_table_ptr(pid);


    // make pointer for the start of page table ( also can be considered the first PTE for the talbe)

    struct Page_Table_Entry *page_table = (struct Page_Table_Entry*)&phys_mem[*page_table_loc_register[pid]];  // pointing to uint8_t (the one that starts the array)

    
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

    struct Page_Table_Entry *pte = &page_table[virtual_frame_number];

    //when should we make the page table? Maybe at the beginning of the process

    printf("Found pte: physical_frame_no: 0x%x,, valid %x\n", pte->physical_frame_number, pte->valid);


    // why start at 0 for i?
    for(int i = MM_ALL_PAGE_TABLES_SIZE_PAGES; i < MM_PHYSICAL_PAGES; i++){
        printf("Considering physical frame %d\n", i);
        
        if(used_pages[i] == 0){
            used_pages[i] = 1;
            if(writable == 0){
                pte->writable = 0;
            } else {
                pte->writable = 1;
            }
                        
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

// loads physical memory given pid and virtual memory
int MM_LoadByte(int pid, uint32_t address, uint8_t *value) {
    // pid -> page table




    uint32_t offset = address & MM_PAGE_OFFSET_MASK;
    uint32_t virtual_frame_number = address >> MM_PAGE_SIZE_BITS;

    // get pte
    struct Page_Table_Entry *page_table = (struct Page_Table_Entry*)&phys_mem[*page_table_loc_register[pid]];
    struct Page_Table_Entry *pte = &page_table[virtual_frame_number];

    //need a physical_pointer
    uint32_t physical_pointer = (pte->physical_frame_number << MM_PAGE_SIZE_BITS) | offset; // | is bitwise
    //used as an index into phys_mem

    return physical_pointer;
} 

int MM_StoreByte(int pid, uint32_t address, uint8_t value) {
	return -1;
}	

int MM_GetStats(int pid, struct MM_Stats *stats) {
	return -1;
}

// Section 2: File for swap, if backed by disk.
FILE *swap_disk_file;





