#ifndef X86_KERNEL_IO_H
#define X86_KERNEL_IO_H

#include <stdint.h>

static inline void Outb(uint16_t port, uint8_t val);
static inline unsigned char inb(uint16_t port);

#endif
