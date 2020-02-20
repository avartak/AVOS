#ifndef KERNEL_X86_IDT_H
#define KERNEL_X86_IDT_H

#include <stdint.h>

#define X86_IDT_NENTRIES            0x100

#define X86_IDT_FLAGS_PRESENT       0x80
#define X86_IDT_FLAGS_DPL0          0
#define X86_IDT_FLAGS_DPL1          0x20
#define X86_IDT_FLAGS_DPL2          0x40
#define X86_IDT_FLAGS_DPL3          0x60

#define X86_IDT_TYPE_TASK           5
#define X86_IDT_TYPE_INTR16         6
#define X86_IDT_TYPE_TRAP16         7
#define X86_IDT_TYPE_INTR32         0xE
#define X86_IDT_TYPE_TRAP32         0xF

struct X86_IDT_Entry{
    uint16_t addr_low;
    uint16_t segment;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t addr_high;
}__attribute__((packed));

struct X86_IDT_Descriptor {
    uint16_t  limit;
    uintptr_t base;
}__attribute__((packed));

extern void X86_IDT_Load(void* descriptor);
extern void X86_IDT_SetupEntry(struct X86_IDT_Entry* entry, uintptr_t address, uint16_t segment, uint8_t type);

#endif
