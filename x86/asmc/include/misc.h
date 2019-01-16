#ifndef ASMC_MISC_H
#define ASMC_MISC_H

#include <stdint.h>

// We define the asm code to be "volatile"
// This is a simple fencing against optimizations of the compiler
// It's less severe than further adding a "memory" clobber
// For now we do it this way
 


static inline void HaltSystem() {

	asm volatile ("halt");

}

// Directly tweaking CR0

static inline void OrCR0Bits(uint32_t bitmap) {

    asm volatile (
        "
        movl %%cr0, %%eax;
        or %0, %%eax
        movl %%eax, %%cr0
        "
        :
        : "r"(bitmap)
        : "%eax"
    )


}

static inline void AndCR0Bits(uint32_t bitmap) {
    
    asm volatile (
        "
        movl %%cr0, %%eax;
        and %0, %%eax
        movl %%eax, %%cr0
        "
        :
        : "r"(bitmap)
        : "%eax"
    )


}

#endif
