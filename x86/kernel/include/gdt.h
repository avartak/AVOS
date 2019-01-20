#ifndef X86_KERNEL_GDT_H
#define X86_KERNEL_GDT_H

#include <stdint.h>

#define KERNEL_CODE_SEG_ACCESS  0x9A
#define KERNEL_DATA_SEG_ACCESS  0x92
#define KERNEL_STACK_SEG_ACCESS 0x96

#define USER_CODE_SEG_ACCESS    0xFA
#define USER_DATA_SEG_ACCESS    0xF2
#define USER_STACK_SEG_ACCESS   0xF6

#define SEG_GRANULARITY         0x0C


#define asm __asm__
#define volatile __volatile__


struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  limit_flags;
    uint8_t  base_high;
}__attribute__((packed));

struct GDTRecord {
    uint16_t  limit;
    uintptr_t base;
}__attribute__((packed));

extern struct GDTEntry gdt[];
extern struct GDTRecord gdtr;

extern void SetupGDTEntry(struct GDTEntry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
extern void SetupGDT();

static inline void LoadGDT(struct GDTRecord* gdtr);

inline void LoadGDT(struct GDTRecord* gdtr) {
    asm volatile ("lgdt %0" : : "m"(*gdtr));
}

extern void LoadKernelSegments();
extern void LoadUserSegments();


#endif
