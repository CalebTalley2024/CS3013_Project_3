# Project 3

- by Caleb Talley, Chris Smith
---

running code: `make ./mm`

Explanation of Project: Same as Instructions
Figure for Page Swapping Algorithms: P3_Figure.png

An explanation of what program was used to simulate load, and briefly how it was used.
• Any results, like a graph of page faults per memory size or page faults per page size.
• An interpretation of the graph and what this means in terms of computer performance.

The data in the graph suggests a huge difference in performance between the two smaller
memory sizes (TINYMEM and LITTLEMEM) and the largest memory size (BIGMEM). This confirms
our intuitions that the more space there is, the less likely it is for a page fault to
occur, making less disk accesses, and therefore increasing performance. Seeing as this
experiment was done on a rather small program (simply using the MM implementation), we
can assume that this gap will only grow as the program complexity grows. That being said,
BIGMEM uses up a lot more memory (512 pages) than LITTLEMEM or TINYMEM (16 and 8 pages 
respectively). So, for systems that do not have a lot of space for memory, sacrificing
performance for more efficient memory allocation is an understandable choice. This
difference in time drops is most likely because TINYMEM and LITTLEMEM have 6 page faults
while BIGMEM has none.

As for the the differences between algorithms, the most surprising result is that the
algorithm with most variance was the "always 2" approach. It could be due to potential
differences in how the swaps are handled in each test.

Tiny and little have a lot of variance for iteration 1, 2, 3. BIGMEM has less overall, highest when random.
The reason for that is 

Time for big is substantially lower than tiny and little.



Test Experiments:

Each test is done three times except for 9 (once for TINYMEM, LITTLEMEM, and BIGMEM):

Test 1: Tests that store and load work properly.
Test 2a: Makes sure that, when memory is full, an error occurs (if swapdisk is not enabled).
Test 2b: Makes sure that, when memory is full, page faults occurs.
Test 3: Ensures that only 4 pid's can be included in the MM.
Test 4: Shows the maximum amount of pid's while also demonstrating a page fault.
Test 5: Tests that read-only pte's do not support the store function.
Test 6: Tests multiple page faults, with maximum pid's, and storing/loading one page.
Test 7: Tests that attempting load on an evicted page will cause a page fault.
Test 8: Automap works.
Test 9 Suite:

- 9a tested the random eviction algorithm
- 9b tested the always two eviction algorithm
- 9c tested the round robin eviction algorithm
- 9d tested the always one of two eviction algorithm (pages 2, 3)
- 9e tested the always one of three eviction algotithm(pages 2, 3, 4)

Test 10: Comparison of page ejection algorithms. Each Test 10 was run three times, but is not
         shown because it updated the same text file name.

When no letter: random
When b: Always two
When c: Round robin
When d: One of two
When e: One of three


Dosbox implementation  was attempted. We however ran into issues. When we tried running an exe file, after doing all the commands in the instructions, the terminal freezes.

Here was the output after running the `DOSBox SVN`:

CS3013 Assignment Pages Allocated 17 Page Faults 15