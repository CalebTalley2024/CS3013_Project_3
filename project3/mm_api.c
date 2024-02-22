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

// Section 2: File for swap, if backed by disk.
FILE *swap_disk_file;

// A single page table entry.


 struct Page_Table {
    struct Page_Table_Entry *ptes[MM_NUM_PTES];
}; 

// A single page table entry.
 struct Page_Table_Entry {
    // pte_page_t page; // What goes here??
    pte_page_t physical_frame_number : 20; 
    pte_page_t valid: 1;
    pte_page_t swapped: 1;
    pte_page_t writable;

};


//Chris's initial idea (not the real one)
//We have an array of page tables. Maybe we can put this array of
//page tables inside physical memory. In this way, we can access
//these page tables by pointing to this part of phys_mem, and then
//accessing the pid part of the array by using the pid #. page_tables[pid]
//start of phys_mem -> page_tables[pid]


// create 4 page table pointers
struct Page_Table *page_tables[MM_MAX_PROCESSES];




int init_mm (){
    // create table
    
    // init pointers for Page Tables
    for(int i = 0; i < MM_MAX_PROCESSES; i++){

        // get 1 page table
        struct Page_Table *page_table = page_tables[i];

        // make sure pointers are going to seperate locations in memory, separated by the size of each page table (num ptes * size_of_pte)    
        //int page_table_size = MM_PTE_SIZE_BYTES * MM_NUM_PTES;

        // page_table -> physical memory location
        //page_table = (void*)phys_mem + i * page_table_size;
    }

    *page_tables = (void*)phys_mem;

    // init used_pages
    for (int i = 0; i<MM_PHYSICAL_PAGES; i++){
        used_pages[i] = 0;
    }

    return 0;
}



// init page table pointers
init_mm();

//////////////////////// By Google Instructor

// struct page_table_entry *start_of_page_table = &phys_mem[page_table_location_register[pid]];
// struct page_table_entry *pte = &start_of_page_table[virtual_frame_number];

// struct page_table_entry* page_table_start = &phys_mem[pid * MM_PAGE_TABLE_SIZE_BYTES];


/////////////////////////





uint8_t used_pages[MM_PHYSICAL_PAGES];

// Per process stats.
struct MM_Stats process_stats[MM_MAX_PROCESSES];


//We are here, we are here, we are here!


// #TODO use pid when getting page table
struct MM_MapResult MM_Map(int pid, uint32_t address, int writable) {  

    // pid  -->  page table
    struct Page_Table * page_table = page_tables[pid];
    
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

    struct page_table_entry *pte = &page_table[virtual_frame_number];

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

int MM_LoadByte(int pid, uint32_t address, uint8_t *value) {
    // pid -> page table




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




