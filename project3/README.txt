# Project 3

- Caleb Talley, Chris Smith

Use this to run: `make ./mm`


Things that still need to be done:

ReadMe
Tests
check that we have everything

Optional
TINY and BIG MEM implementation

DOSBox

Page Ejection Algs




Questions


should I page fault for trying to store in a "read only" disk (eg. making a copy that is writable)?

valid bit vs swapped bit

is it fine to not have virtual page number? (no virtual memory slot?)

virtual frame number = virtual page number?

make_resident, eject phys 1 pid 0 vp 0 pp 1

from what I can tell this means..

phys 1: Refers to physical frame number 1.
pid 0: Refers to Process ID 0.
vp 0: Refers to Virtual Page number 0.
pp 1: Refers to Physical Page number 1.

are phys and pp not the same thing?

do I count the page table page as an allocated page when it comes to `pages_allocated`?


// do the below stats look right. It's far less than what you had in your example for part 2

Output: `Stats for pid 0: 4 pages_allocated 5 page_faults`

For tests, we just have to split the example  input's into separate tests, correct. Were you expecting this in one text file, or multiple test txt files in a folder?


Will there be extra credit for Project 4, if so, how much

how points work here.

