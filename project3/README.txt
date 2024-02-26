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

cat test1.txt | ./mm > test1.txt


0 page faults when running BIGMEM - 10 pages_allocated 0 page_faults

- max virtual address: set bound
- DOSBox
- testing



Simplest way:
Just type things, copy into a text file, done.

Better way:
Create an input test file:
test1_in.txt:
0,map,0x1000,1
0,store,0x1023,123
0,load,0x1023,0

Then derive output:
cat test1_in.txt | ./mm > test1_out.txt

Also add:
test1_description.txt with
Map, store, load test in one page.


