#ifndef X86_KERNEL_IO_H
#define X86_KERNEL_IO_H

#include <stdint.h>

/*

These are the C wrappers for X86 instructions that read from and write to ports.
Both functions take a 16-bit port address
The 'b' in Inb and Outb says that the data that is read/written is byte-sized
The Outb function takes a byte as the second argument. This byte is written to the port
The Inb function reads a byte from the port and returns its value

*/

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
