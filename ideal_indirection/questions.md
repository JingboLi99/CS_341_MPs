/**
 * ideal_indirection
 * CS 341 - Spring 2023
 */
For all of these problems we want you to answer them in your own words to the best of your abilities. You are still allowed to refer to resources like lecture notes, the coursebook, Google, your peers and your friends. We care a lot more that you learn from these questions then being able to do them 100% yourself. So if you get stuck ... "ask your neighbor" :)


For these questions please read the [coursebook](http://cs241.cs.illinois.edu/coursebook/Ipc#translating-addresses)
* What is a 'virtual address'?
* What is a 'physical address'?
* Can two processes read and write to the same virtual address?
* What is an MMU?
* What is a Page Table?
  * What is the difference between a page and a frame?
    * Which is larger?
  * What is a virtual page number?
  * What is an offset?
* How much physical memory (in GB) can a 32 bit machine address? Explain.
* How much physical memory (in GB) can a 64 bit machine address? Explain.
* On a 32 bit machine with 2kb pages and 4byte entries how big would a Page Table (single tier) have to be to address all of physical memory? Explain.
* On a 64 bit machine with 8kb pages and 8byte entries how big a Page Table (single tier) have to be to address all of physical memory? Explain.
  * If you had 32GB of ram would this work out? Why or why not?
* What is a multi-level page table?
  * What problem do they solve ("All problems in computer science can be solved by another level of indirection")?
  * But what problem do they introduce ("... except of course for the problem of too many indirections")?
    * How can we solve this problem / What is a TLB ("... and this is problem is solved by the cache")?
    * Provide me one example where you use indirection in computer-science and one example where you use indirection in real life.
* What is a page fault?
* What is a cache miss?

For these questions please read mmu.h, page_table.h and tlb.h:

* How is our page table implemented?
  * What does each entry point to?
* How many levels of page tables are we working with?
* How many entries does a page table have?
* How many bytes is a page table?
* How much memory (in GB) can we address with our multi-level page table (for a single process)? Explain.

* How is our TLB implemented (Note that it is an LRU Cache)
* What does LRU mean?
* Which element is the most recently used element?
* Which element is the least recently used element?
* What is the runtime of TLB_get_physical_address()?
* What is the runtime of TLB_add_physical_address()?
* Why do TLB_get_physical_address() and TLB_add_physical_address() need a double pointer?
* Why do TLB_get_physical_address() and TLB_add_physical_address() need both a virtual address and pid?

THIS IS NOT GRADED! But it will really help you get to know virtual memory and our simulation to complete the assignment.
