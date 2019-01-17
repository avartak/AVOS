#ifndef X86_ASMC_IO_H
#define X86_ASMC_IO_H

#include <stdint.h>

// We define the asm code to be "volatile"
// This is a simple fencing against optimizations of the compiler
// It's less severe than further adding a "memory" clobber
// For now we do it this way
 

// Port IO

static inline void Outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

#endif
