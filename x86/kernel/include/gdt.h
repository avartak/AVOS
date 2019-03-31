#ifndef X86_KERNEL_GDT_H
#define X86_KERNEL_GDT_H

#include <kernel/include/common.h>

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

#define GDT_TSS_SEG               0x2B

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

struct TSS_Segment {
    uint16_t previous_task_link;
    uint16_t reserved0;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved4;
    uint16_t cs;
    uint16_t reserved5;
    uint16_t ss;
    uint16_t reserved6;
    uint16_t ds;
    uint16_t reserved7;
    uint16_t fs;
    uint16_t reserved8;
    uint16_t gs;
    uint16_t reserved9;
    uint16_t ldt_seg_selector;
    uint16_t reserved10;
    uint16_t reserved11;
    uint16_t iomap_base_address;
    uint8_t  ioport_map[0x2000];
}__attribute__((packed));

extern struct GDT_Entry      GDT_entries[];
extern struct GDT_Descriptor GDT_desc;
extern struct TSS_Segment    TSS_seg;

extern void   GDT_SetupEntry(struct GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
extern void   GDT_Load(struct GDT_Descriptor* desc);
extern void   GDT_LoadKernelSegments();
extern void   TSS_LoadTaskRegister(uint16_t desc);

extern void   GDT_Initialize();

#endif
