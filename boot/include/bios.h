#ifndef BOOT_BIOS_H
#define BOOT_BIOS_H

#include <boot/include/defs.h>

struct BIOS_Registers {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint16_t  ds;
    uint16_t  es;
    uint16_t  ss;
    uint16_t flags;
}__attribute__((packed));

extern void BIOS_Interrupt(uint32_t interrupt, struct BIOS_Registers* regs);
extern void BIOS_ClearRegistry(struct BIOS_Registers* regs);

#endif

