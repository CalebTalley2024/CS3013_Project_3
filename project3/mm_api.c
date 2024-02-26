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

// Can you read this, Caleb?
// int roundRobin = 0;
// this will be for the round robin style eviction algorithm
// should this just be a global variable,
// or should this be included in each page table?

int roundRobinIndex = 1; // variable used for round robin eviction implementation
int swap_alg = 3;

LL_PF leastRecentlyUsed;

// gets random integer btw 0 and max     2 and 10
int get_rand_int(int min, int max)
{
    srand(time(NULL));                             // Seed the random number generator
    int rand_int = rand() % (max - min + 1) + min; // Generate a random integer in [min, max]
    return rand_int;
}

// uint8_t has smaller range than 'int', only takes up 1 byte vs 4 bytes
// range of 8 bit (1 byte): 0 - 255: 11111111 in binary = 255
// first 4 page frames are reserved for process page tables
uint8_t phys_mem[MM_PHYSICAL_MEMORY_SIZE_BYTES];

// travk page faults of each process @chris
int num_page_faults[MM_MAX_PROCESSES]; // total num page faults per process
int pages_allocated[MM_MAX_PROCESSES]; // totl num pages every (not just currently) allocated in phy_mem per process

// Section 2: File for swap, if backed by disk.
FILE *disk;

uint8_t automap_enabled = 0;

uint8_t swap_enabled = 0;
// int pages_allocated = 1; // assume that the first page (for pte's is)

// page table location reister: holds location of the start of each processes'page table
struct Page_Table_Entry *page_table_loc_register[MM_MAX_PROCESSES];

void init_page_table_loc_register()
{
    for (int pid = 0; pid < MM_MAX_PROCESSES; pid++)
    {
        page_table_loc_register[pid] = NULL;
    }
}
// @caleb TODO might have to rework if swapping
int add_page_table_ptr(int pid)
{
    // make register for pid point to base + offset
    // base = location of the first page in phy_mem
    // offset  = size of a page * pid               (location where the page table should be)
    // We are assuming that each Page table is the same size, that way we can use pid to move the pointer by an offset each time
    int offset = MM_PAGE_TABLE_SIZE_BYTES * pid;
    page_table_loc_register[pid] = (struct Page_Table_Entry *)(&phys_mem[offset]); // point register to memory slot
    return 0;
}

uint8_t used_pages[MM_PHYSICAL_PAGES]; // default value is 0

// Per process stats.
struct MM_Stats process_stats[MM_MAX_PROCESSES];

// init_used_pages();
// init_used_pages (used_pages);
// We are here, we are here, we are here!
// #TODO use pid when getting page table
struct MM_MapResult MM_Map(int pid, uint32_t address, int writable)
{
    // make page table pointer for this pid
    add_page_table_ptr(pid);
    // make pointer for the start of page table ( also can be considered the first PTE for the talbe)
    struct Page_Table_Entry *page_table = page_table_loc_register[pid]; // pointing to uint8_t (the one that starts the array)
    // start here
    struct MM_MapResult ret = {0};
    static char message[128];
    // printf("Virtual pointer = %x\n", address);
    // we must convert from a virtual pointer to a virtual page
    // and an offset by doing the two things below
    // uint32_t offset = address & MM_PAGE_OFFSET_MASK;
    uint32_t VPN = address >> MM_PAGE_SIZE_BITS; // Virtual Page Number = Virtual Frame Number = VPN
    printf("address >> MM_PAGE_SIZE_BITS = %d >> %d\n", address, MM_PAGE_SIZE_BITS);
    printf("Virtual frame number = %x\n", VPN);
    // printf("Offset = %x\n", offset);
    struct Page_Table_Entry *pte = &page_table[VPN];
    // when should we make the page table? Maybe at the beginning of the process
    //  printf("Found pte: physical_frame_no: 0x%x,, valid %x\n", pte->PFN, pte->valid);
    //  why start at 0 for i? -->  page table is at 0

    used_pages[0] = 1; // page table page is used
    for (int PFN = MM_ALL_PAGE_TABLES_SIZE_PAGES; PFN < MM_PHYSICAL_PAGES; PFN++)
    { // start adding to pages that after after the 'page table' pages
        // printf("Considering physical frame %d\n", PFN);
        if (used_pages[PFN] == 0)
        {
            used_pages[PFN] = 1;
            pte->writable = writable;
            // pte->VPN = VPN;
            // printf("PFN: %d\n",PFN);
            pte->PFN = PFN; // Page Frame Number = Physical Frame Number = PFN
            pte->valid = 1;
            pages_allocated[pid]++;
            // printf("pte %p -> writable: %d\n", (void *)pte,pte-> writable);
            // add_node(leastRecentlyUsed, PFN);
            ret.new_mapping = 1;
            ret.physical_frame = PFN;
            return ret;
        }
    }
    // Page is NOT found if rest of code is executed
    if (swap_enabled)
    {
        // page fault if there are no pages for us to alloate the page
        page_fault(pte, pid, VPN);

        int PFN = pte->PFN;
        used_pages[PFN] = 1;
        pte->writable = writable;
        // pte->VPN = VPN;
        pte->PFN = PFN;
        pte->valid = 1;
        pages_allocated[pid]++;

        // add_node(leastRecentlyUsed, pte ->PFN); // push to the top of linked list
        ret.new_mapping = 1;
        ret.physical_frame = PFN;
        return ret;
    }
    else
    {
        sprintf(message, "Map failed: out of memory");
        ret.message = message;
        ret.error = 1;
        return ret;
    }
}

// loads memory address' value into value variable
int MM_LoadByte(int pid, uint32_t address, uint8_t *value)
{

    uint32_t VPN = address >> MM_PAGE_SIZE_BITS;
    // check if register for pid does not point to anything ((nil)/null/0x0)
    // only map if automap is enabled
    if (page_table_loc_register[pid] == NULL)
    { // change to 'CHECK'?
        // printf("this only points to null: %p", (void *) page_table_loc_register[pid]);

        if (automap_enabled == 1)
        {
            // map address so that it can be used to load/store
            // keep writable
            // also makes valid bit 1
            MM_Map(pid, address, 1); //
        }
        else
        {
            printf("Load: pid register doesnt point to anything\n");
            return 1;
        }
    }
    // printf("value at phys_mem: %d\n",phys_mem[*page_table_loc_register[pid]]);
    // get pte (need Mapping for this)
    struct Page_Table_Entry *page_table = page_table_loc_register[pid];
    struct Page_Table_Entry *pte = &page_table[VPN];

    if (pte->valid == 0) // page was never mapped
    {
        if (automap_enabled == 1)
        {
            // map address so that it can be used to load/store
            // keep writable
            // also makes valid bit 1
            MM_Map(pid, address, 1); //
        }
        else
        {
            printf("Load:Error valid bit == 0, \n");
            return 2;
        }
    }
    if (pte->swapped == 1) // page in disk
    {

        if (swap_enabled)
        {
            // page fault if there are no pages for us to alloate the page
            page_fault(pte, pid, VPN);
            
        }
        else
        { // not swapped
            printf("Load failed: page not in memory");
        }
    }
    // add_node(leastRecentlyUsed, pte -> PFN); // push to the top of linked list
    // set value to the value stored in pte
    *value = pte->value;
    // add_node();
    //  printf("%p",(void*)pte);

    return 0; // idk how main will know about value, but lets see
}

// stores value into memory via PTE pointer
int MM_StoreByte(int pid, uint32_t address, uint8_t value)
{
    // pid -> page table
    uint32_t VPN = address >> MM_PAGE_SIZE_BITS;
    // check if register for pid does not point to anything ((nil)/null/0x0)
    // only map if automap is enabled
    if ((page_table_loc_register[pid] == NULL))
    { // change to 'CHECK'?
        // CHECK(page_table_loc_register[pid]);
        // printf("this only points to null: %p", (void *) page_table_loc_register[pid]);

        if (automap_enabled == 1)
        {
            // map address so that it can be used to load/store
            // keep writable
            // also makes valid bit 1
            MM_Map(pid, address, 1); //
        }
        else
        {
            printf("Store: pid register doesnt point to anything\n");
            return 1;
        }
    }
    // get pte
    struct Page_Table_Entry *page_table = page_table_loc_register[pid];
    struct Page_Table_Entry *pte = &page_table[VPN];
    // update pte's value if the page is writable
    // printf("pte %p -> writable: %d\n", (void *)pte,pte-> writable);
    if (pte->valid == 0) // page was never mapped
    {
        if (automap_enabled == 1)
        {
            // map address so that it can be used to load/store
            // keep writable
            // also makes valid bit 1
            MM_Map(pid, address, 1); //
        }
        else
        {
            printf("Load:Error valid bit == 0, \n");
            return 2;
        }
    }
    if (pte->swapped == 1)
    {
        if (swap_enabled)
        {
            // page fault if there are no pages for us to alloate the page
            page_fault(pte, pid, VPN);
        }
        else
        { // not swapped
            printf("Load failed: page not in memory");
        }
    }
    // add_node(leastRecentlyUsed, pte -> PFN); // push to the top of linked list
    if (pte->writable)
    {
        pte->value = value;
    }
    // else
    // {
    //     // printf("Store Error: cannot write to 'read only' page\n");
    //     // printf("store failed\n");
    //     return 1;
    // }

    return 0;
}

uint32_t MM_PageSize() { return MM_PAGE_SIZE_BYTES; }

// enables automapping
void MM_AutoMap()
{
    automap_enabled = 1;
}

// enables swapping
void MM_SwapOn()
{
    swap_enabled = 1;
}

// for the given pid, find the number of allocated pages and number
// of page faults. I am not sure how we can return both at once...
int MM_GetStats(int pid, struct MM_Stats *stats)
{

    // int pages_allocated = 1;  // start at one (b/c page table page)
    // sum up all values (+1 means a page is allocated, +0, which does not affect sum means page is not allocated)
    // for (int i = MM_ALL_PAGE_TABLES_SIZE_PAGES; i < MM_PHYSICAL_PAGES; i++) pages_allocated  += used_pages[i];
    // printf("Stats for pid %d: %d pages allocated and %d page faults", pid, pages_allocated, num_page_faults);
    stats->pages_allocated = pages_allocated[pid];
    stats->page_faults = num_page_faults[pid];
    return 0;

    // ok, cool. what does used_pages include? is that the number of total pages for a specific pid?
    // or is it just the current number of pages in phy_mem
}

// @ chris
// int PFN = 0;
// for extra credit, PFN can be changed to be just the 0th index
// also, maybe there is a way to track how long it has been since
// a page has been accessed, in which case we choose the largest one
// linked list of index numbers?
// most recent at the front, when page is accessed
// it is put at the front, and if it was already in
// the list somewhere else it is removed from that spot
// I imagine that whenever load_byte, store_byte, and
// MM_Map is called that this linked list idea should be called

/*page fault instances
- Map: try to map page, but no open pages
- Load: try to load page, but valid bit = 0 (not in page table)
*/
// pte_in: pte that will replace pte_out in the page table
int page_fault(struct Page_Table_Entry *pte_in, int pid, pte_page_t VPN)
{ //

    int PFN = get_PFN();

    int offset = PFN * MM_PTE_SIZE_BYTES; // get the offset for the random index
    struct Page_Table_Entry *pte_old = (struct Page_Table_Entry *)&phys_mem[offset];

    printf("make_resident, eject phys %d pid %d vp %d pp %d\n", PFN, pid, VPN, PFN);

    CHECK((disk = fopen("disk.txt", "r+")) != NULL); // make disk point to dist.txt file (read + write permissions)

    // if (disk == NULL)
    // {
    //     perror("Error opening file");
    //     exit(EXIT_FAILURE);
    // }

    // SEEK_SET: Beginning of the file.
    fseek(disk, offset, SEEK_SET);                        // position file position to top of txt file // <TODO> will have to change this for each page
    pte_page_t buffer[MM_PTE_SIZE_BYTES];                 // buffer to help swap phys_mem[offset] and disk
    memcpy(buffer, &phys_mem[offset], MM_PTE_SIZE_BYTES); // phys_mem[offset] --> buffer

    CHECK(fread(&phys_mem[offset], MM_PTE_SIZE_BYTES, 1, disk) == 1); // phys_mem[offset] <-- disk

    fwrite(buffer, MM_PTE_SIZE_BYTES, 1, disk); // write pte_old's page, into disk, buffer --> disk

    // update pte_out, which is going to disk
    pte_old->PFN = -1; // -1; not assigned to physical frame number
    // pte_old -> valid = 0; //
    pte_old->swapped = 1; // is in disk

    // printf("'pte_in -> PFN' is next\n");
    // update pte_in, the pte being swapped into page table

    pte_in->PFN = PFN;
    pte_in->valid = 1;   //
    pte_in->swapped = 0; // is in disk

    // Map Page
    num_page_faults[pid]++; // add to the number of page faults

    fclose(disk); // close page
    return 0;
}

// gets PFN with certain swapping algorithm
// uses global variable swap_alg
int get_PFN()
{
    pte_page_t PFN;
    pte_page_t max_idx = MM_PHYSICAL_PAGES - 1; // for case 3
    pte_page_t swap_index_2 = 2;                // for case 2

    switch (swap_alg)
    {
    case 1: // 1: Random
        PFN = get_rand_int(MM_ALL_PAGE_TABLES_SIZE_PAGES, MM_PHYSICAL_PAGES);
        break;
    case 2: // 2: PFN always equals 2
        PFN = swap_index_2;
        break;
    case 3:                                        // round robin
        PFN = (++roundRobinIndex) % (max_idx) + 1; // corrected variable name
        break;
    case 4:                        //We tried getting this to work, but we ended up with core dumps and the deadline is soon
        PFN = get_LRU_Page();
        break;
    default: // error
        PFN = -1;
        break;
    }
    return PFN;
}

// gets Least recently used page in page table
int get_LRU_Page()
{
    int PFN = leastRecentlyUsed.head -> PFN;
    leastRecentlyUsed.head = leastRecentlyUsed.head->next;
    return PFN;
}

// int add_node(LL_PF list, int PFN)
// {

//     // make new node
//     Node *newNode = malloc(sizeof(Node));
//     if (newNode == NULL)
//     {
//         return -1;
//     }
//     newNode->PFN = PFN;
//     // newNode.next -> NULL;
//     if (list.head == NULL)
//     { // if first node, make it the head
//         list.head = newNode;
//         return 0;
//     }

//     // if the function makes it this far, there is already an head

//     Node *currentNode = malloc(sizeof(Node));
//     currentNode = list.head;
//     Node *previous = NULL;
//     int i = 0;
//     while (currentNode != NULL)
//     {
//         if (i == 0 && currentNode->PFN == PFN)
//         { // case when page frame that is least recently used is being accessed again,
//             // requiring us to move this PFN to the end of the linked list. Therefore
//             // we remove the head, and place it at the end of the linked list.
//             list.head = currentNode->next;
//             currentNode = currentNode->next;
//         }
//         else if (currentNode->PFN == PFN)
//         { // This covers every case where the page frame is being accessed again and
//           // is not the head. We can simply point previous at currentNode.next so that
//           // we skip over the node being removed and placed at the end.
//             previous->next = currentNode->next;
//             free(currentNode);
//             // currentNode = currentNode.next;
//             // previous = currentNode.next;
//         }
//         else
//         { // if the page is not being accessed again (PFN is not the same as node PFN) we simply move along
//             previous = currentNode;
//         }
//         i++;
//         currentNode = currentNode->next;
//         printf("Previous and Current Node PFN: %d, %d", previous->PFN, currentNode->PFN);
//     }
//     previous->next = newNode;
//     newNode->next = NULL;
//     return 0;
// }