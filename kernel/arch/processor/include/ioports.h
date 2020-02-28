#ifndef KERNEL_IOPORTS_H
#define KERNEL_IOPORTS_H

#include <stdint.h>

extern void     Outb(uint16_t port, uint8_t  value);
extern void     Outw(uint16_t port, uint16_t value);
extern void     Outd(uint16_t port, uint32_t value);

extern uint8_t  Inb(uint16_t port);
extern uint16_t Inw(uint16_t port);
extern uint32_t Ind(uint16_t port);

#endif
