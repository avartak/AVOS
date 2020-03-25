#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

#define IDT_NENTRIES            0x100

#define IDT_FLAGS_PRESENT       0x80
#define IDT_FLAGS_DPL0          0
#define IDT_FLAGS_DPL1          0x20
#define IDT_FLAGS_DPL2          0x40
#define IDT_FLAGS_DPL3          0x60

#define IDT_TYPE_TASK           5
#define IDT_TYPE_INTR16         6
#define IDT_TYPE_TRAP16         7
#define IDT_TYPE_INTR32         0xE
#define IDT_TYPE_TRAP32         0xF

struct IDT_Entry{
    uint16_t addr_low;
    uint16_t segment;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t addr_high;
}__attribute__((packed));

struct IDT_Descriptor {
    uint16_t  limit;
    uintptr_t base;
}__attribute__((packed));

extern void IDT_Load(void* descriptor);
extern void IDT_SetupEntry(struct IDT_Entry* entry, uintptr_t address, uint16_t segment, uint8_t type);

#endif
