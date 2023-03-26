/**
 * ideal_indirection
 * CS 341 - Spring 2023
 */
#pragma once
#include <stdint.h>

/**
 * In this assignment, we will be simulating a 32-bit architecture machine.
 * The virtual memory space and physical memory space of this simulated machine
 * can then be fully represented with a 32-bit integer, which we've typedef-ed
 * as an addr32.
 *
 * Note that your actual VM environment may or may not be 32-bit architecture.
 * Therefore, you will need to translate the physical addresses of this
 * simulated machine into memory addresses of your actual machine.
 */
typedef uint32_t addr32;
