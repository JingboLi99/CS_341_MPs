/**
 * ideal_indirection
 * CS 341 - Spring 2023
 */
#include "mmu.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
// ** Helper functions
addr32 getBaseAddress(addr32 va){ //mask off least sig bit based on 4KB page size
    return va & 0xFFFFF000;
}
mmu *mmu_create() {
    mmu *my_mmu = calloc(1, sizeof(mmu));
    my_mmu->tlb = tlb_create();
    return my_mmu;
}

void mmu_read_from_virtual_address(mmu *this, addr32 virtual_address,
                                   size_t pid, void *buffer, size_t num_bytes) {
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);
    // TODO: Implement me!
    //Context switching: Make sure the mmu is for the current pid we are checking for
    if (pid != this->curr_pid){
        this->curr_pid = pid;
        tlb_flush(&(this->tlb));
    }
    // Check that address is in a segment and get that segment if it exists
    vm_segmentations * cur_proc_segments = this->segmentations[pid];
    // vm_segmentation *seg = NULL;
    bool in_segment = address_in_segmentations(cur_proc_segments, virtual_address);
    if (!in_segment){
        mmu_raise_segmentation_fault(this);
        return;
    }
    else{
        vm_segmentation* seg = find_segment(cur_proc_segments, virtual_address);
        int can_read = seg->permissions & READ;
        if (!can_read){
            mmu_raise_segmentation_fault(this);
            return;
        }
    }
    // Check if address is in TLB
    addr32 base_page_add = getBaseAddress(virtual_address);
    page_table_entry* cur_pte = tlb_get_pte(&this->tlb, base_page_add);
    if (!cur_pte){ //if address is not in TLB: ie need to find from PDE
        mmu_tlb_miss(this);
        // Check if address is in PDE
        page_directory * cur_page_dir = this->page_directories[pid];
        addr32 pde_base_add = (virtual_address) >> (VIRTUAL_ADDR_SPACE - NUM_OFFSET_BITS);
        page_directory_entry* cur_pde = &cur_page_dir->entries[pde_base_add];
        if (!cur_pde->present){ //directory Not present: copy from disk and store in physical memory
            mmu_raise_page_fault(this); //raise page fault
            cur_pde->base_addr = (ask_kernel_for_frame(NULL) >> NUM_OFFSET_BITS);
            read_page_from_disk((page_table_entry *) cur_pde);
            cur_pde->present = 1;
            cur_pde->read_write = 1;
            cur_pde->user_supervisor =0; //note
        }
        //pde is present
        page_table * cur_page_table = (page_table *) get_system_pointer_from_pde(cur_pde);
        addr32 pte_base_add = (virtual_address & 0x003FF000) >> NUM_OFFSET_BITS; //get middle 10 bits
        cur_pte = &cur_page_table->entries[pte_base_add];
    }
    //if current pte is not present
    if (!cur_pte->present){
        mmu_raise_page_fault(this); //raise page fault
        cur_pte->base_addr = (ask_kernel_for_frame(cur_pte) >> NUM_OFFSET_BITS);
        read_page_from_disk(cur_pte);
        cur_pte->present = 1;
        cur_pte->read_write = 1;
        cur_pte->user_supervisor = 1; //note
    }
    
    cur_pte->accessed = 1;
    void * phy_page = (void *)get_system_pointer_from_pte(cur_pte);
    addr32 offset = virtual_address & 0xFFF;
    memcpy(buffer, phy_page + offset, num_bytes);

    tlb_add_pte(&this->tlb, base_page_add, cur_pte);
}

void mmu_write_to_virtual_address(mmu *this, addr32 virtual_address, size_t pid,
                                  const void *buffer, size_t num_bytes) {
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);
    // TODO: Implement me!
    //Context switching: Make sure the mmu is for the current pid we are checking for
    if (pid != this->curr_pid){
        this->curr_pid = pid;
        tlb_flush(&(this->tlb));
    }
    // Check that address is in a segment and get that segment if it exists
    vm_segmentations * cur_proc_segments = this->segmentations[pid];
    // vm_segmentation *seg = NULL;
    bool in_segment = address_in_segmentations(cur_proc_segments, virtual_address);
    if (!in_segment){
        mmu_raise_segmentation_fault(this);
        return;
    }
    else{
        vm_segmentation* seg = find_segment(cur_proc_segments, virtual_address);
        int can_write = seg->permissions & WRITE;
        if (!can_write){
            mmu_raise_segmentation_fault(this);
            return;
        }
    }
    // Check if address is in TLB
    addr32 base_page_add = getBaseAddress(virtual_address);
    page_table_entry* cur_pte = tlb_get_pte(&this->tlb, base_page_add);
    if (!cur_pte){ //if address is not in TLB: ie need to find from PDE
        mmu_tlb_miss(this);
        // Check if address is in PDE
        page_directory * cur_page_dir = this->page_directories[pid];
        addr32 pde_base_add = (virtual_address) >> (VIRTUAL_ADDR_SPACE - NUM_OFFSET_BITS);
        page_directory_entry* cur_pde = &cur_page_dir->entries[pde_base_add];
        if (!cur_pde->present){ //directory Not present: copy from disk and store in physical memory
            mmu_raise_page_fault(this); //raise page fault
            cur_pde->base_addr = (ask_kernel_for_frame(NULL) >> NUM_OFFSET_BITS);
            read_page_from_disk((page_table_entry *) cur_pde);
            cur_pde->present = 1;
            cur_pde->read_write = 1;
            cur_pde->user_supervisor = 0; //note
        }
        //pde is present
        page_table * cur_page_table = (page_table *) get_system_pointer_from_pde(cur_pde);
        addr32 pte_base_add = (virtual_address & 0x003FF000) >> NUM_OFFSET_BITS; //get middle 10 bits
        cur_pte = &cur_page_table->entries[pte_base_add];
    }
    //if current pte is not present
    if (!cur_pte->present){
        mmu_raise_page_fault(this); //raise page fault
        cur_pte->base_addr = (ask_kernel_for_frame(cur_pte) >> NUM_OFFSET_BITS);
        read_page_from_disk(cur_pte);
        cur_pte->present = 1;
        cur_pte->read_write = 1;
        cur_pte->user_supervisor = 1; //note
    }
    
    cur_pte->accessed = 1;
    cur_pte->dirty = 1; //has been written to when set

    void * phy_page = (void *)get_system_pointer_from_pte(cur_pte);
    addr32 offset = virtual_address & 0xFFF;
    memcpy(phy_page + offset, buffer, num_bytes);

    tlb_add_pte(&this->tlb, base_page_add, cur_pte);
}

void mmu_tlb_miss(mmu *this) {
    this->num_tlb_misses++;
}

void mmu_raise_page_fault(mmu *this) {
    this->num_page_faults++;
}

void mmu_raise_segmentation_fault(mmu *this) {
    this->num_segmentation_faults++;
}

void mmu_add_process(mmu *this, size_t pid) {
    assert(pid < MAX_PROCESS_ID);
    addr32 page_directory_address = ask_kernel_for_frame(NULL);
    this->page_directories[pid] =
        (page_directory *)get_system_pointer_from_address(
            page_directory_address);
    page_directory *pd = this->page_directories[pid];
    this->segmentations[pid] = calloc(1, sizeof(vm_segmentations));
    vm_segmentations *segmentations = this->segmentations[pid];

    // Note you can see this information in a memory map by using
    // cat /proc/self/maps
    segmentations->segments[STACK] =
        (vm_segmentation){.start = 0xBFFFE000,
                          .end = 0xC07FE000, // 8mb stack
                          .permissions = READ | WRITE,
                          .grows_down = true};

    segmentations->segments[MMAP] =
        (vm_segmentation){.start = 0xC07FE000,
                          .end = 0xC07FE000,
                          // making this writeable to simplify the next lab.
                          // todo make this not writeable by default
                          .permissions = READ | EXEC | WRITE,
                          .grows_down = true};

    segmentations->segments[HEAP] =
        (vm_segmentation){.start = 0x08072000,
                          .end = 0x08072000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[BSS] =
        (vm_segmentation){.start = 0x0805A000,
                          .end = 0x08072000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[DATA] =
        (vm_segmentation){.start = 0x08052000,
                          .end = 0x0805A000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[TEXT] =
        (vm_segmentation){.start = 0x08048000,
                          .end = 0x08052000,
                          .permissions = READ | EXEC,
                          .grows_down = false};

    // creating a few mappings so we have something to play with (made up)
    // this segment is made up for testing purposes
    segmentations->segments[TESTING] =
        (vm_segmentation){.start = PAGE_SIZE,
                          .end = 3 * PAGE_SIZE,
                          .permissions = READ | WRITE,
                          .grows_down = false};
    // first 4 mb is bookkept by the first page directory entry
    page_directory_entry *pde = &(pd->entries[0]);
    // assigning it a page table and some basic permissions
    pde->base_addr = (ask_kernel_for_frame(NULL) >> NUM_OFFSET_BITS);
    pde->present = true;
    pde->read_write = true;
    pde->user_supervisor = true;

    // setting entries 1 and 2 (since each entry points to a 4kb page)
    // of the page table to point to our 8kb of testing memory defined earlier
    for (int i = 1; i < 3; i++) {
        page_table *pt = (page_table *)get_system_pointer_from_pde(pde);
        page_table_entry *pte = &(pt->entries[i]);
        pte->base_addr = (ask_kernel_for_frame(pte) >> NUM_OFFSET_BITS);
        pte->present = true;
        pte->read_write = true;
        pte->user_supervisor = true;
    }
}

void mmu_remove_process(mmu *this, size_t pid) {
    assert(pid < MAX_PROCESS_ID);
    // example of how to BFS through page table tree for those to read code.
    page_directory *pd = this->page_directories[pid];
    if (pd) {
        for (size_t vpn1 = 0; vpn1 < NUM_ENTRIES; vpn1++) {
            page_directory_entry *pde = &(pd->entries[vpn1]);
            if (pde->present) {
                page_table *pt = (page_table *)get_system_pointer_from_pde(pde);
                for (size_t vpn2 = 0; vpn2 < NUM_ENTRIES; vpn2++) {
                    page_table_entry *pte = &(pt->entries[vpn2]);
                    if (pte->present) {
                        void *frame = (void *)get_system_pointer_from_pte(pte);
                        return_frame_to_kernel(frame);
                    }
                    remove_swap_file(pte);
                }
                return_frame_to_kernel(pt);
            }
        }
        return_frame_to_kernel(pd);
    }

    this->page_directories[pid] = NULL;
    free(this->segmentations[pid]);
    this->segmentations[pid] = NULL;

    if (this->curr_pid == pid) {
        tlb_flush(&(this->tlb));
    }
}

void mmu_delete(mmu *this) {
    for (size_t pid = 0; pid < MAX_PROCESS_ID; pid++) {
        mmu_remove_process(this, pid);
    }

    tlb_delete(this->tlb);
    free(this);
    remove_swap_files();
}
