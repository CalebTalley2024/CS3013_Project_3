#ifndef MM_API_H__
#define MM_API_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

// Helper macro for quick error checking.
#define CHECK(x)	do { if (!(x)) { fprintf(stderr, "%s:%d CHECK failed: %s, errno %d %s\n", __FILE__, __LINE__, #x, errno, strerror(errno)); abort(); } } while(0)

// Maximum processes allowed.
// Valid values of 'pid' arguments are 0, 1, 2, 3.
#define MM_MAX_PROCESSES			4


// Support a couple of different memory layouts; 'big' for backing actual workloads,
// 'little' for easier testing, 'tiny' for testing the lower limits.
#if !defined(BIGMEM) && !defined(TINYMEM) && !defined(LITTLEMEM)
# if defined(WPI_OS_PROJECT_3)	// Given to DOSBox compilation
#  define BIGMEM
# else
#  define LITTLEMEM	// sensible default.
# endif
#endif

#if defined(BIGMEM)
typedef uint32_t pte_page_t;
#define MM_PAGE_SIZE_BITS			12		// 4kB pages
#define MM_PHYSICAL_MEMORY_SIZE_SHIFT		21		// 2MB physical
#define MM_PROCESS_VIRTUAL_MEMORY_SIZE_SHIFT	26		// 32MB virtual
#define MM_BUF_CHUNK_SIZE			64		// malloc/fread size
#define MM_PTE_SIZE_BYTES			4		// Each page table entry is 32 bits.

#elif defined(TINYMEM)
typedef uint8_t pte_page_t;
#define MM_PAGE_SIZE_BITS			4``		// 16b pages (fits 4 PTEs)
#define MM_PHYSICAL_MEMORY_SIZE_SHIFT		(MM_PAGE_SIZE_BITS + 3)	// 8 pages physical mem
#define MM_PROCESS_VIRTUAL_MEMORY_SIZE_SHIFT	(MM_PHYSICAL_MEMORY_SIZE_SHIFT + 1)	// virtual = 2x physical
#define MM_BUF_CHUNK_SIZE			(1 << MM_PAGE_SIZE_BITS)
#define MM_PTE_SIZE_BYTES			1		// Each page table entry is 1 byte.


#elif defined(LITTLEMEM)
typedef uint32_t pte_page_t;
#define MM_PAGE_SIZE_BITS			12		// 4kB pages
#define MM_PHYSICAL_MEMORY_SIZE_SHIFT		(MM_PAGE_SIZE_BITS + 2)	// 4 pages physical mem. 1 stores all PTEs for all procs.
#define MM_PROCESS_VIRTUAL_MEMORY_SIZE_SHIFT	(MM_PHYSICAL_MEMORY_SIZE_SHIFT + 1)	// virtual = 2x physical
#define MM_BUF_CHUNK_SIZE			64		// malloc/fread size
#define MM_PTE_SIZE_BYTES			4		// Each page table entry is 32 bits.
#endif

#define MM_PAGE_SIZE_BYTES			(1 << MM_PAGE_SIZE_BITS)
#define MM_PAGE_OFFSET_MASK			(MM_PAGE_SIZE_BYTES - 1)

#define MM_PHYSICAL_MEMORY_SIZE_BYTES		(1 << MM_PHYSICAL_MEMORY_SIZE_SHIFT)
#define MM_PROCESS_VIRTUAL_MEMORY_SIZE_BYTES	(1 << MM_PROCESS_VIRTUAL_MEMORY_SIZE_SHIFT)
#define MM_MAX_SWAP_SIZE_BYTES			(MM_PROCESS_VIRTUAL_MEMORY_SIZE_BYTES * MM_MAX_PROCESSES)
#define MM_PHYSICAL_PAGES			(MM_PHYSICAL_MEMORY_SIZE_BYTES / MM_PAGE_SIZE_BYTES)
#define MM_SWAP_PAGES				(MM_MAX_SWAP_SIZE_BYTES / MM_PAGE_SIZE_BYTES)
#define MM_NUM_PTES				(MM_PROCESS_VIRTUAL_MEMORY_SIZE_BYTES / MM_PAGE_SIZE_BYTES)
#define MM_PAGE_TABLE_SIZE_BYTES		(MM_NUM_PTES * MM_PTE_SIZE_BYTES)

// A simple way to store page tables is to just use the start of physical memory for them,
// in a few un-evictable pages.
#define MM_ALL_PAGE_TABLES_SIZE_PAGES		((MM_PAGE_TABLE_SIZE_BYTES * MM_MAX_PROCESSES + MM_PAGE_SIZE_BYTES - 1) / MM_PAGE_SIZE_BYTES)

// Per process statistics for page faults and allocations.
struct MM_Stats {
	uint32_t pages_allocated;
	uint32_t page_faults;
};

// Results of a MM_Map() function call.
struct MM_MapResult {
	int error;
	const char *message;	// Does not need to be freed.
	int new_mapping;
	uint32_t physical_frame;
};

// Map a page of memory for the requested process.
// If 'writable' is non-zero, the page is mapped read/write. Otherwise, the
// page is mapped read-only. 'address' is the virtual address requested,
// not the page number. If the page corresponding to 'address' is unmapped,
// create a pagetable entry. If the page is already mapped, update the
// permission bits of the mapping to adhere to the new 'writable' setting.
struct MM_MapResult MM_Map(int pid, uint32_t address, int writable);

// Set up automatic page mappings. This removes the requirement that pages
// be mapped before accessing. If AutoMap is on for, and a previously unmapped
// page is accessed, map the page as read-write then perform the access as
// expected. If memory is exhausted, abort().
// NB: This is to connect our memory manager to other things, and is not
// a common/realistic feature of a real memory manager.
void MM_AutoMap();

// Enable Swap Whether to enable Swap in the memory manager. This should
// open a file on the filesystem, and allow storage of virtual pages
// in the file backing.
void MM_SwapOn();

// Returns the current page size. Does not need to be dynamically
// configurable at all, this is just a compile time constant.
// This is useful to allow clients to know how to divide up their address
// space into differently permissioned pages.
uint32_t MM_PageSize();

// Load a byte from the specified address.
// 0 is returned for a valid load operation. If the page is not mapped,
// and AutoMap is not enabled, return -1.
int MM_LoadByte(int pid, uint32_t address, uint8_t *value);

// Store a byte in the specified address. 
// 0 is returned for a valid store operation. If the page is not mapped,
// is mapped read-only, or AutoMap is not enabled, return -1.
// The memory should be modified ONLY if the return value is zero.
int MM_StoreByte(int pid, uint32_t address, uint8_t value);

// Return statistics for the pid.
// We want to keep track of the total allocated pages, and the number
// of page faults that are occurring.
int MM_GetStats(int pid, struct MM_Stats *stats);


// @caleb custom functions
// A single page table entry.
// pte_page_t page; // What goes here??
 struct Page_Table_Entry {
    pte_page_t physical_frame_number : 20;  // 20 bits
    pte_page_t valid: 1; 
    pte_page_t swapped: 1;
    pte_page_t writable : 1;

    uint8_t value: 8;
};
int add_page_table_ptr(int pid);

void init_page_table_loc_register();

int get_rand_int(int min, int max);

// void page_fault(struct Page_Table_Entry * pte_in, struct Page_Table_Entry * pte_out, int PFN_out);
int page_fault(struct Page_Table_Entry * pte_in);
// struct Page_Table_Entry {
//     // pte_page_t page; // What goes here??
//     pte_page_t physical_frame_number : 20; 
//     pte_page_t valid: 1;
//     pte_page_t swapped: 1;
//     pte_page_t writable;

// };

#ifdef __cplusplus
}
#endif

#endif	// MM_API_H__
