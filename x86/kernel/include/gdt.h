#ifndef X86_KERNEL_GDT_H
#define X86_KERNEL_GDT_H

#include <stdint.h>

#include <x86/asmc/include/pmode.h>

#define KERNEL_CODE_SEG_ACCESS  0x9A
#define KERNEL_DATA_SEG_ACCESS  0x92
#define KERNEL_STACK_SEG_ACCESS 0x96

#define USER_CODE_SEG_ACCESS    0xFA
#define USER_DATA_SEG_ACCESS    0xF2
#define USER_STACK_SEG_ACCESS   0xF6

#define SEG_GRANULARITY         0x0C

extern struct GDTEntry gdt[7];
extern struct GDTRecord gdtr;

extern void SetupGDTEntry(struct GDTEntry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
extern void SetupGDT();

#endif
