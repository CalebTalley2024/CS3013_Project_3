# Project 3

- Caleb Talley, Chris Smith

Use this to run: `make ./mm`

askf or grading on Project 2
Things that still need to be done:

ReadMe ( explain, explain each test, etc)
Tests
check that we have everything
try having multiple processes
teammate evaluation


Optional
TINY and BIG MEM implementation

DOSBox

Page Ejection Algs


0 page fautls when running BIGMEM


Questions

Thanks for your comments

I meant to ask, if it fine to not have an explicit virtual memory array, meaning that virtual addresses are used, but not explicity associated with a virtual memory array. I dont have that at the moment I was fine when it came to running examples

In the doc, the following is said: "In LITTLEMEM mode, the page table for every process
can fit in a single page. "

if this is the case, why does MM_PAGE_SIZE_BITS = 12 bits while MM_PTE_SIZE_BYTES = 4 Bytes? Wouldnt that mean that the PTEs, which make up each page table, are larger than a page?