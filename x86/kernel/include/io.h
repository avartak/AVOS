#ifndef X86_KERNEL_IO_H
#define X86_KERNEL_IO_H

#include <stdint.h>

extern inline void   Outb(uint16_t port, uint8_t val);
extern inline uint8_t Inb(uint16_t port);

inline void Outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline uint8_t Inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

#endif
