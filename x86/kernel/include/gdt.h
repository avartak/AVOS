#ifndef X86_KERNEL_GDT_H
#define X86_KERNEL_GDT_H

#include <stdint.h>

#define GDT_KERN_CODE_SEG_ACCESS  0x9A
#define GDT_KERN_DATA_SEG_ACCESS  0x92

#define GDT_USER_CODE_SEG_ACCESS  0xFA
#define GDT_USER_DATA_SEG_ACCESS  0xF2

#define GDT_TSS_SEG_TYPE_ACCESS   0x89

#define GDT_SEG_GRANULARITY       0x0C
#define GDT_TSS_GRANULARITY       0x00

#define GDT_KERN_CODE_SEG         0x08
#define GDT_KERN_DATA_SEG         0x10

#define GDT_USER_CODE_SEG         0x18
#define GDT_USER_DATA_SEG         0x20

#define GDT_KERN_TSS_SEG          0x28

#define asm __asm__
#define volatile __volatile__


struct GDT_Entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  limit_flags;
    uint8_t  base_high;
}__attribute__((packed));

struct GDT_Descriptor {
    uint16_t  limit;
    uintptr_t base;
}__attribute__((packed));

extern struct GDT_Entry      GDT_entries[];
extern struct GDT_Descriptor GDT_desc;

extern void GDT_SetupEntry(struct GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
extern void GDT_Initialize();

static inline void GDT_Load(struct GDT_Descriptor* desc);

inline void GDT_Load(struct GDT_Descriptor* desc) {
    asm volatile ("lgdt %0" : : "m"(*desc));
}

extern void GDT_LoadKernelSegments();

#endif
