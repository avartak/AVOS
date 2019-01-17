#ifndef X86_KERNEL_IO_H
#define X86_KERNEL_IO_H

#include <stdint.h>

extern inline void Outb(uint16_t port, uint8_t val);
extern inline unsigned char inb(uint16_t port);

#endif
