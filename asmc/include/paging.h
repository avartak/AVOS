#ifndef ASMC_PAGING_H
#define ASMC_PAGING_H

// We define the asm code to be "volatile"
// This is a simple fencing against optimizations of the compiler
// It's less severe than further adding a "memory" clobber
// For now we do it this way
 


// Paging

static inline void EnablePGBitInCR0() {

    asm volatile (
        "
        movl %%cr0, %%eax;
        or $0x80000000, %%eax
        movl %%eax, %%cr0
        "
        :
        :
        : "%eax"
    )

}

static inline void LoadPageDirectory(uint32_t* pdt) {

    asm volatile (
        "
        movl %0, %%eax;
        movl %%eax, %%cr3
        "
        :
        : "m"(*pdt)
        : "%eax"
    )

}

#endif
