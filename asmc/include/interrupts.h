#ifndef ASMC_INTERRUPTS_H
#define ASMC_INTERRUPTS_H

#include <stdint.h>

// We define the asm code to be "volatile"
// This is a simple fencing against optimizations of the compiler
// It's less severe than further adding a "memory" clobber
// For now we do it this way
 


// Some singular instructions

static inline void ClearInterrupts() {

	asm volatile ("cli");

}

static inline void EnableInterrupts() {

	asm volatile ("sti");

}

struct IDTRecord {
    uint16_t  limit;
    uintptr_t    base;
} __attribute__((packed));

static inline void LoadIDT(struct IDTRecord* idtr) {

    asm volatile ("lidt %0" : : "m"(*idtr));

}

#endif
