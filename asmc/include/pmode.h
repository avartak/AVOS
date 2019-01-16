#ifndef ASMC_PMODE_H
#define ASMC_PMODE_H

#include <stdint.h>

// We define the asm code to be "volatile"
// This is a simple fencing against optimizations of the compiler
// It's less severe than further adding a "memory" clobber
// For now we do it this way
 


// Protected mode

static inline void EnablePEBitInCR0() {

    asm volatile (
		"
		movl %%cr0, %%eax;
		or $1, %%eax
		movl %%eax, %%cr0
        "
        :
        :
        : "%eax"
    )

}

static inline void DisablePEBitInCR0() {

    asm volatile (
        "
        movl %%cr0, %%eax;
        and $FE, %%eax
        movl %%eax, %%cr0
        "
        :
        :
        : "%eax"
    )

}

struct GDTRecord {
    uint16_t  limit;
    uintptr_t    base;
} __attribute__((packed));

static inline void LoadGDT(struct GDTRecord* gdtr) {

    asm volatile ("lgdt %0" : : "m"(*gdtr));


}

#endif
