#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "mm_api.h"
///////////////////////////////////////////////////////////////////////////////
// All implementation goes in this file.                                     //
///////////////////////////////////////////////////////////////////////////////

// gets random integer btw 0 and max     2 and 10
int get_rand_int(int min, int max) {
    srand(time(NULL)); // Seed the random number generator
    int rand_int = rand() % (max - min + 1) + min; // Generate a random integer in [min, max]
    return rand_int;
}

// uint8_t has smaller range than 'int', only takes up 1 byte vs 4 bytes
// range of 8 bit (1 byte): 0 - 255: 11111111 in binary = 255
// first 4 page frames are reserved for process page tables
uint8_t phys_mem[MM_PHYSICAL_MEMORY_SIZE_BYTES];

// A single page table entry.
// pte_page_t page; // What goes here??
//  struct Page_Table_Entry {
//     pte_page_t physical_frame_number : 20;  // 20 bits
//     pte_page_t valid: 1; 
//     pte_page_t swapped: 1;
//     pte_page_t writable: 1;

//     uint8_t value: 8;
// };

//travk page faults of each process @chris
int p_fault_counter[MM_MAX_PROCESSES];


// Section 2: File for swap, if backed by disk.
FILE *disk;

//Chris's initial idea (not the real one)
//We have an array of page tables. Maybe we can put this array of
//page tables inside physical memory. In this way, we can access
//these page tables by pointing to this part of phys_mem, and then
//accessing the pid part of the array by using the pid #. page_tables[pid]
//start of phys_mem -> page_tables[pid]


// create 4 page table pointers
// struct Page_Table *page_tables[MM_MAX_PROCESSES];

uint8_t automap_enabled = 0;

uint8_t swap_enabled = 0;

// page table location reister: holds location of the start of each processes'page table
uint8_t *page_table_loc_register[MM_MAX_PROCESSES];

void init_page_table_loc_register(){
    for (int pid; pid < MM_MAX_PROCESSES; pid++){
        page_table_loc_register[pid] = NULL;

    }
}
// @caleb TODO might have to rework if swapping
int add_page_table_ptr(int pid){ 
    // make register for pid point to base + offset
    // base = location of the first page in phy_mem
    // offset  = size of a page * pid               (location where the page table should be)
    // We are assuming that each Page table is the same size, that way we can use pid to move the pointer by an offset each time
    int offset = MM_PAGE_TABLE_SIZE_BYTES* pid;
    // printf("\nhello");
    
    page_table_loc_register[pid] =  (uint8_t*)(&phys_mem + offset); // point register to memory slot

    // printf("new loc %p: \n", page_table_loc_register[pid+1]);

    
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
    add_page_table_ptr(pid);

    // make pointer for the start of page table ( also can be considered the first PTE for the talbe)

    struct Page_Table_Entry *page_table = (struct Page_Table_Entry*)&phys_mem[*page_table_loc_register[pid]];  // pointing to uint8_t (the one that starts the array)

    //start here
    struct MM_MapResult ret = {0};
    // static char message[128];

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


    // why start at 0 for i? -->  page table is at 0
    for(int PFN = MM_ALL_PAGE_TABLES_SIZE_PAGES; PFN < MM_PHYSICAL_PAGES; PFN++){
        printf("Considering physical frame %d\n", PFN);
        
        if(used_pages[PFN] == 0){
            used_pages[PFN] = 1;
            if(writable == 0){
                pte->writable = 0;
            } else {
                pte->writable = 1;
            }
                        
            pte->physical_frame_number = PFN;
            pte->valid = 1;
            ret.new_mapping = 1;
            ret.physical_frame = PFN;
            return ret;
        }
    }
    if (swap_enabled){
        // page fault if there are no pages for us to alloate the page
        int PFN_old = get_rand_int(MM_ALL_PAGE_TABLES_SIZE_PAGES, MM_PHYSICAL_PAGES); 
        int offset_old = PFN_old * MM_PTE_SIZE_BYTES; // get the offset for the random index
        struct Page_Table_Entry *pte_old = (struct Page_Table_Entry*) &phys_mem[offset_old];

        page_fault(pte, pte_old,PFN_old);

        ret.new_mapping = 1;
        ret.physical_frame = PFN_old;
        return ret;
    }
    else{
        sprintf(message, "Map failed: out of memory");
        ret.message = message;
        ret.error = 1;
        return ret;
    }
} 

// loads memory address' value into value variable
int MM_LoadByte(int pid, uint32_t address, uint8_t *value) {

    // printf("\n\nhelllo");
    uint32_t virtual_frame_number = address >> MM_PAGE_SIZE_BITS;

    // check if register for pid does not point to anything ((nil)/null/0x0)
    // only map if automap is enabled
    if ((page_table_loc_register[pid] == NULL)){  // change to 'CHECK'?
        // printf("this only points to null: %p", (void *) page_table_loc_register[pid]);
        printf("pid register doesnt point to anything\n");

        if(automap_enabled == 1){
            
            // map address so that it can be used to load/store
            // keep writable
            MM_Map(pid, address, 1); // 
        }
        else{return 1;}
    }
    
    // printf("value at phys_mem: %d\n",phys_mem[*page_table_loc_register[pid]]);
    // get pte (need Mapping for this)
    struct Page_Table_Entry *page_table = (struct Page_Table_Entry*)&phys_mem[*page_table_loc_register[pid]];
    struct Page_Table_Entry *pte = &page_table[virtual_frame_number];

    // set value to the value stored in pte
    *value = pte -> value;
    // printf("%p",(void*)pte);

    return 0; // idk how main will know about value, but lets see
} 

// stores value into memory via PTE pointer
int MM_StoreByte(int pid, uint32_t address, uint8_t value) {

    // pid -> page table
    uint32_t virtual_frame_number = address >> MM_PAGE_SIZE_BITS;

    // check if register for pid does not point to anything ((nil)/null/0x0)
    // only map if automap is enabled
    if ((page_table_loc_register[pid] == NULL)){ // change to 'CHECK'?
    // CHECK(page_table_loc_register[pid]);
        // printf("this only points to null: %p", (void *) page_table_loc_register[pid]);
        printf("pid register doesnt point to anything\n");

        if(automap_enabled == 1){
            
            // map address so that it can be used to load/store
            // keep writable
            MM_Map(pid, address, 1); // 
        }
        else{return 1;}
    }

    // get pte
    struct Page_Table_Entry *page_table = (struct Page_Table_Entry*)&phys_mem[*page_table_loc_register[pid]];
    struct Page_Table_Entry *pte = &page_table[virtual_frame_number];

    // update pte's value
    pte -> value = value;
    return 0;
}	






uint32_t MM_PageSize() { return MM_PAGE_SIZE_BYTES; }

// enables automapping
void MM_AutoMap() {
	automap_enabled = 1;
}

void MM_SwapOn() {
    swap_enabled = 1;
}


//for the given pid, find the number of allocated pages and number
//of page faults. I am not sure how we can return both at once...

int MM_GetStats(int pid, struct MM_Stats *stats) {
    int totalPages, numberOfPageFaults;
    totalPages = -1;
    numberOfPageFaults = -1;


    printf("For pid: %d\n", pid);
    printf("There are %d total allocated pages\n", totalPages);
    printf("And %d page faults", numberOfPageFaults);
	return 0;

    //ok, cool. what does used_pages include? is that the number of total pages for a specific pid?
    //or is it just the current number of pages in phy_mem
    
}




    /*page fault instances
    
    - Map (swap_type = m): try to map page, but no open pages
    - Load (swap_type = l): try to load page, but valid bit = 0 (not in page table)
    - Store (swap_type = s): try to write to a read only page ( writable = 0)
    */ 
    
    // if there are no page tables available, do a page fault

    // pte_in: pte that will replace pte_out in the page table
    // pte_out: pte that will be placed in disk

void page_fault(struct Page_Table_Entry * pte_in, struct Page_Table_Entry * pte_out, int PFN_out){
    printf("page faulting needed \n");
    
    disk = fopen("disk.txt","w"); // make disk point to dist.txt file (read + write permissions)

    if (disk == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // from MEMORY to DISK

    // if (swap_type == "m"){  
    /* int fseek(FILE *stream, long offset, int whence);
     stream: pointer
     offset: # of bytes offset from whence position
     whence:
        - SEEK_SET: Beginning of the file.
        - SEEK_CUR: Current position indicator.
        - SEEK_END: End of the file.
    */
    // page size in bytes: MM_PTE_SIZE_BYTES
    int offset = PFN_out * MM_PTE_SIZE_BYTES;

    fseek(disk,offset,SEEK_SET); // position file position to top of txt file // TODO will have to change this for each page

    // size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
    fwrite(&phys_mem[offset], offset, 1, disk); // write page, that pte_out refers to, into disk

    // update pte_out, which is going to disk
    pte_out -> physical_frame_number = -1; // -1; not assigned to physical frame number
    pte_out -> valid = 0; // 
    pte_out -> swapped = 1; // is in disk
    
    //update pte_in, the pte being swapped into page table
    
    pte_in -> physical_frame_number = PFN_out;
    pte_in -> valid = 1; // 
    pte_in -> swapped = 0; // is in disk
    
    // used_pages[PFN_out] == 0; // reset used page at the PFN

    // resume execution
    // }


    // from DISK to Memory (swapped = 1)
    // fread(&phys_mem[offset], offset, 1, disk); // write what is at the pointer into disk


    // return 0;


}

    






