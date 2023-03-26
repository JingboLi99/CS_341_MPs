/**
 * ideal_indirection
 * CS 341 - Spring 2023
 */
/**
 * tlb.h - An implementation of the Translation Lookaside Buffer (TLB).
 */
#pragma once
#include "page_table.h"
#include "types.h"
#include <stdlib.h>

// Max number of items that the tlb can hold.
#define MAX_NODES 16

/**
 * The tlb in our simulation is an LRU cache. Therefore, the tlb is implemented
 * as a linked list, since it makes it easy to keep track of which entry was
 * least recently used. Each tlb struct is a node in the linked list.
 */
typedef struct tlb {
    /**
     * The virtual address with the offset removed. We use this because
     * virtual addresses with the same virtual page number will map to the
     * same physical memory frame.
     */
    addr32 base_virtual_addr;
    /**
     * The page table entry that stores the base address of the physical
     * memory frame that corresponds to the virtual page addressed in
     * base_virtual_addr.
     */
    page_table_entry *entry;
    /**
     * The next pointer for the linked list. The tail will be the LRU entry in
     * the tlb.
     */
    struct tlb *next;
} tlb;

/**
 * Allocates and returns a new tlb structure.
 */
tlb *tlb_create();

/**
 * Checks if the tlb contains the virtual page addressed by base_virtual_addr.
 *
 * @param head - A pointer to a tlb pointer. This allows us to update the head
 *               of the tlb.
 * @param base_virtual_addr - The base address of a virtual page.
 * @return the pte that corresponds to base_virtual_addr if it is in the tlb.
 *         If not, then this will return NULL.
 */
page_table_entry *tlb_get_pte(tlb **head, addr32 base_virtual_addr);

/**
 * Adds the corresponding (base_virtual_addr, entry) pair to the tlb.
 * If the tlb is at its capacity, then it will evict the least recently used
 * (LRU) item.
 *
 * @param head - a pointer to a tlb pointer. This allows us to update the head
 *               of the tlb.
 * @param base_virtual_addr - The base address of a virtual page.
 * @param entry - The page table entry that points to the physical memory frame
 *                that corresponds to the virtual page at base_virtual_addr.
 */
void tlb_add_pte(tlb **head, addr32 base_virtual_addr, page_table_entry *entry);

/**
 * Clears the TLB's data.
 */
void tlb_flush(tlb **head);

/**
 * Frees the TLB.
 */
void tlb_delete(tlb *tlb);
