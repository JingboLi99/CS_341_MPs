/**
 * ideal_indirection
 * CS 341 - Spring 2023
 */
/**
 * segments.h - defines the various segments in a virtual memory space.
 */
#pragma once
#include "types.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * The definition of a segmentation within your virtual memory space.
 */
typedef struct {
    /* The starting point of your segment */
    addr32 start;
    /* The ending point of your segment */
    addr32 end;
    /**
     * The set of access permissions on this segment. You will want to
     * perform bit-wise operations with the enum permissions defined
     * below
     */
    addr32 permissions;
    /* Denotes whether the segment grows down or up */
    bool grows_down;
} vm_segmentation;

/**
 * An enum to denote the different types of segments that can exist in our
 * simulated virtual memory space.
 */
typedef enum segments {
    STACK,
    MMAP,
    HEAP,
    BSS,
    DATA,
    TEXT,
    TESTING, // This segment is made up for testing purposes.
    NUM_SEGMENTS
} segments;

/**
 * The list of segmentations that can be in a virtual memory space. This is
 * the struct that represents the entirety of your virtual memory. Note that
 * this is implemented as an array of segments.
 */
typedef struct {
    vm_segmentation segments[NUM_SEGMENTS];
} vm_segmentations;

/**
 * The different types of permissions stored within the permission entry in
 * each segmentation.
 */
typedef enum permissions { READ = 0x1, WRITE = 0x2, EXEC = 0x4 } permissions;

/**
 * Increases the size of a segment by num_pages * bytes_per_page.
 *
 * @param segmentations - The virtual memory space whose segment is getting
 *                        increased.
 * @param segment - The target segment whose size is getting increased.
 * @param num_pages - The number of pages to increase the size.
 */
void grow_segment(vm_segmentations *segmentations, segments segment,
                  size_t num_pages);

/**
 * Reduces the size of a segment by num_pages * bytes_per_page.
 *
 * @param segmentations - The virtual memory space whose segment is getting
 *                        reduced.
 * @param segment - The target segment whose size is getting reduced.
 * @param num_pages - The number of pages to reduce the size.
 */
void shrink_segment(vm_segmentations *segmentations, segments segment,
                    size_t num_pages);

/**
 * Checks if a particular address exists within the the segments of a virtual
 * memory space (not every address is used in a virtual memory space).
 *
 * @param segmentations - The virtual memory space to search for an address.
 * @param address - The address to look for.
 * @return true if the address exists in the space, false otherwise.
 */
bool address_in_segmentations(vm_segmentations *segmentations, addr32 address);

/**
 * Finds the segment within a virtual memory space that contains a given
 address.
 *
 * @param segmentations - The virtual memory space to search for an address.
 * @param address - The address to look for.
 * @return the segmentation that contains the given address. Returns NULL if
           the address does not exist in any of the segments.
 */
vm_segmentation *find_segment(vm_segmentations *segmentations, addr32 address);
