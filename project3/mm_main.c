#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mm_api.h"


int main(int argc, char **argv)
{
	printf("Page = %d bytes, Page table size = %d bytes. All page tables = %d pages\n",
		MM_PAGE_SIZE_BYTES, MM_PAGE_TABLE_SIZE_BYTES, MM_ALL_PAGE_TABLES_SIZE_PAGES);
	printf("MM_PTE_SIZE_BYTES %d\n", MM_PTE_SIZE_BYTES);
	printf("MM_MAX_PROCESSES %d\n", MM_MAX_PROCESSES);
	printf("MM_ALL_PAGE_TABLES_SIZE_PAGES %d\n", MM_ALL_PAGE_TABLES_SIZE_PAGES);
	printf("MM_PAGE_SIZE_BYTES %d\n", MM_PAGE_SIZE_BYTES);
	printf("MM_PHYSICAL_MEMORY_SIZE_BYTES %d\n", MM_PHYSICAL_MEMORY_SIZE_BYTES);
	printf("MM_PROCESS_VIRTUAL_MEMORY_SIZE_BYTES %d\n", MM_PROCESS_VIRTUAL_MEMORY_SIZE_BYTES);
	printf("MM_MAX_SWAP_SIZE_BYTES %d\n", MM_MAX_SWAP_SIZE_BYTES);
	printf("\n");


	FILE *input = NULL;
	if (argc == 2) {
		CHECK((input = fopen(argv[1], "r")) != NULL);
	}

	while (1) {
		ssize_t nread;
		char linebuf[128];
		char *line = linebuf;
		size_t linesz = sizeof(linebuf) - 1;

		int pid;
		char *op;
		uint32_t address;
		uint8_t value;
		char *p;

		int rc;
		int scanned = 0;

		printf("Instruction? ");

		if ((nread = getline(&line, &linesz, input ? input : stdin)) < 0) {
			break;
		}

		// Terrible parsing because sscanf is terrible.
		while (nread > 0 && (line[nread - 1] == '\n' || line[nread - 1] == '\r')) {
			line[nread-- - 1] = 0;
		}
		p = line;
		for (char *c = line; ; c++) {
			int done = *c == '\n' || *c == 0;
			if (*c == ',' || done) {
				*c = 0;
				if ((scanned == 0 && (rc = sscanf(p, "%d", &pid)) == 1) ||
					(scanned == 1 && (op = p)) ||
					(scanned == 2 && (rc = sscanf(p, "%x", &address)) == 1) ||
					(scanned == 3 && (rc = sscanf(p, "%hhu", &value)) == 1)) {
					scanned++;
					p = c+1;
				}
			}
			if (done) break;
		}
					

		if (strcmp(line, "swap_mem") == 0) {
			printf("Swap enabled, backed by memory\n");
			MM_SwapOn(0);
		} else if (strcmp(line, "swap_disk") == 0) {
			printf("Swap enabled, backed by disk\n");
			MM_SwapOn(1);
		} else if (strcmp(line, "automap") == 0) {
			printf("Automap enabled\n");
			MM_AutoMap();
		} else if (scanned == 4) {
			if (pid < 0 || pid >= MM_MAX_PROCESSES) {
				printf("Invalid pid %d\n", pid);
			} else if (strcmp(op, "map") == 0) {
				struct MM_MapResult mr = MM_Map(pid, address, !!value); 
				if (mr.error) {
					printf("Map failed\n");
				} else if (mr.new_mapping) {
					printf("Put page for PID %d virtual frame %u into physical frame %u\n",
							pid, address >> MM_PAGE_SIZE_BITS, mr.physical_frame);
				} else {
					printf("Updating permissions for PID %d virtual page %u (frame %u)\n",
							pid, address >> MM_PAGE_SIZE_BITS, mr.physical_frame);
				}
				if (mr.message) {
					printf("Map: %s\n", mr.message);
				}
			} else if (strcmp(op, "load") == 0) {
				if ((rc = MM_LoadByte(pid, address, &value)) != 0) {
					printf("load failed\n");
				} else {
					printf("Virtual address %x contains value %u\n", address, value);
				}
			} else if (strcmp(op, "store") == 0) {
				if ((rc = MM_StoreByte(pid, address, value)) != 0) {
					printf("store failed\n");
				} else {
					printf("Stored value %u at virtual address %x\n", value, address);
				}
			} else if (strcmp(op, "stats") == 0) {
				struct MM_Stats stats;
				if ((rc = MM_GetStats(pid, &stats)) != 0) {
					printf("MM_GetStats failed: returned %d\n", rc);
				} else {
					printf("Stats for pid %d: %u pages_allocated %u page_faults\n",
							pid, stats.pages_allocated, stats.page_faults);
				}
			} else {
				printf("Unknown operation: '%s'\n", op);
			}
		} else {
			printf("Invalid input\n");
		}
	}

	CHECK(input == NULL || fclose(input) == 0);
	fflush(stdout);
	fflush(stderr);

	return 0;
}



