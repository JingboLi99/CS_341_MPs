/**
 * ideal_indirection
 * CS 341 - Spring 2023
 */
/**
 * kernel.h - Implementation of a simulated kernel.
 */
#pragma once
#include "page_table.h"
#include "types.h"
#include <stdbool.h>
#include <stdlib.h>

#define NUM_PHYSICAL_PAGES 250
// 1MB = 250 4kb pages
#define PHYSICAL_MEMORY_SIZE (PAGE_SIZE * NUM_PHYSICAL_PAGES)

/**
 * Memory pool that represents all of physical memory. Assume that your machine
 * is sad and you only have 1MB of physical memory.
 */
extern char physical_memory[PHYSICAL_MEMORY_SIZE]
    __attribute__((aligned(PAGE_SIZE)));

/**
 * Mapping to determine which page_table_entry owns which page. Note that these
 * page table entries may come from different processes.
 */
extern page_table_entry *entry_mapping[NUM_PHYSICAL_PAGES];

/**
 * Returns the physical address of the next chunk to allocate if possible,
 * these will be in PAGE_SIZE chunks aligned to PAGE_SIZE bytes. If physical
 * memory is filled up, this function will automatically swap one of the memory
 * frames to disk. Note that page directories and page tables will not get
 * swapped to disk in this simulation (this is not true for virtual memory in
 * general).
 *
 * @param entry - The page table entry that will point to this frame. Pass in
 *                NULL if the frame is pointed by something else.
 * @return The physical address of the simulated machine. NULL if there are no
 *         more frames to give out.
 */
addr32 ask_kernel_for_frame(page_table_entry *entry);

/**
 * Frees up a frame of memory that is no longer in use.
 *
 * @param base_addr - The base address of the frame of memory to be freed.
 */
void return_frame_to_kernel(void *base_addr);

/**
 * Removes the swap file of the frame pointed by entry.
 *
 * @param entry - The page table entry that points to the frame whose swap file
 *                is to be deleted.
 */
void remove_swap_file(page_table_entry *entry);

/**
 * Removes all swap files.
 */
void remove_swap_files();

/**
 * Writes a page of physical memory to disk. This frees up a page of physical
 * memory that can be reused.
 *
 * @param entry - The page table entry whose referred frame is to be written
 *                to disk.
 */
void write_page_to_disk(page_table_entry *entry);

/**
 * Reads a page of physical memory from disk. This is used when the page table
 * entry points to a frame of memory that has been swapped to disk, and the
 * page table entry is now pointing to a valid physical memory frame.
 *
 * @param entry - The page table entry which points to the swapped frame.
 */
void read_page_from_disk(page_table_entry *entry);

/**
 * These functions allow us to convert the physical addresses of the simulated
 * machine into physical addresses of our actual machine. You will need to
 * use these functions before performing any actual reads or writes to the
 * data structures in the simulation.
 */

/**
 * Converts the base address of the memory frame pointed by the page table entry
 * into an address used by the machine.
 *
 * @param entry - The page table entry that points to the memory frame.
 */
void *get_system_pointer_from_pte(page_table_entry *entry);

/**
 * Converts the base address of the page table pointed by the page directory
 * entry into an address used by the machine.
 *
 * @param entry - The page directory entry that points to the page table.
 */
void *get_system_pointer_from_pde(page_directory_entry *entry);

/**
 * Converts an address of the simulated physical memory into an address used
 * by the machine.
 *
 * @param address - The address of the simulated physical memory that needs
 *                  to be converted.
 */
void *get_system_pointer_from_address(addr32 address);
