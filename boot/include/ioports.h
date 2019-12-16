#ifndef BOOT_IOPORTS_H
#define BOOT_IOPORTS_H

#include <stdint.h>

#define asm __asm__
#define volatile __volatile__


static inline void    IOPorts_Outb(uint16_t port, uint8_t val);
static inline uint8_t IOPorts_Inb (uint16_t port);

inline void IOPorts_Outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline uint8_t IOPorts_Inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif

