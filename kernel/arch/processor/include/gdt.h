#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <stdint.h>

#define GDT_FLAGS_PMODE32       4
#define GDT_FLAGS_4GB           8

#define GDT_FLAGS_ACCESSED      1
#define GDT_FLAGS_READWRITE     2
#define GDT_FLAGS_DIRCONFORMING 4
#define GDT_FLAGS_EXECUTABLE    8
#define GDT_FLAGS_CODEORDATA    0x10
#define GDT_FLAGS_PRIV_RING0    0
#define GDT_FLAGS_PRIV_RING1    0x20
#define GDT_FLAGS_PRIV_RING2    0x40
#define GDT_FLAGS_PRIV_RING3    0x60
#define GDT_FLAGS_PRESENT       0x80

#define GDT_SEGLIMIT_4GB        0xFFFFF

#define GDT_NENTRIES            6
#define GDT_ENTRY_NULL          0
#define GDT_ENTRY_KERN_CODE     1
#define GDT_ENTRY_KERN_DATA     2
#define GDT_ENTRY_USER_CODE     3
#define GDT_ENTRY_USER_DATA     4
#define GDT_ENTRY_TSS           5

#define GDT_SEG_NULL            (GDT_ENTRY_NULL*8)
#define GDT_SEG_KERN_CODE       (GDT_ENTRY_KERN_CODE*8)
#define GDT_SEG_KERN_DATA       (GDT_ENTRY_KERN_DATA*8)
#define GDT_SEG_USER_CODE       (GDT_ENTRY_USER_CODE*8)
#define GDT_SEG_USER_DATA       (GDT_ENTRY_USER_DATA*8)
#define GDT_SEG_TSS             (GDT_ENTRY_TSS*8)

#define GDT_RPL0                0
#define GDT_RPL1                1
#define GDT_RPL2                2
#define GDT_RPL3                3

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

struct TSS {
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
}__attribute__((packed));

extern void GDT_Load(void* descriptor);
extern void GDT_LoadKernelSegments();
extern void GDT_SetupEntry(struct GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
extern void TSS_LoadTaskRegister(uint16_t selector);

#endif
