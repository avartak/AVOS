#ifndef KERNEL_X86_IOPORTS_H
#define KERNEL_X86_IOPORTS_H

#include <stdint.h>

extern void     X86_Outb(uint16_t port, uint8_t  value);
extern void     X86_Outw(uint16_t port, uint16_t value);
extern void     X86_Outd(uint16_t port, uint32_t value);

extern uint8_t  X86_Inb(uint16_t port);
extern uint16_t X86_Inw(uint16_t port);
extern uint32_t X86_Ind(uint16_t port);

#endif
