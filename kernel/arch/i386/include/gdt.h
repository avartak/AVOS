#ifndef KERNEL_X86_GDT_H
#define KERNEL_X86_GDT_H

#include <stdint.h>

#define X86_GDT_FLAGS_PMODE32       4
#define X86_GDT_FLAGS_4GB           8

#define X86_GDT_FLAGS_ACCESSED      1
#define X86_GDT_FLAGS_READWRITE     2
#define X86_GDT_FLAGS_DIRCONFORMING 4
#define X86_GDT_FLAGS_EXECUTABLE    8
#define X86_GDT_FLAGS_CODEORDATA    0x10
#define X86_GDT_FLAGS_PRIV_RING0    0
#define X86_GDT_FLAGS_PRIV_RING1    0x20
#define X86_GDT_FLAGS_PRIV_RING2    0x40
#define X86_GDT_FLAGS_PRIV_RING3    0x60
#define X86_GDT_FLAGS_PRESENT       0x80

#define X86_GDT_SEGLIMIT_4GB        0xFFFFF

#define X86_GDT_NENTRIES            6
#define X86_GDT_ENTRY_NULL          0
#define X86_GDT_ENTRY_KERN_CODE     1
#define X86_GDT_ENTRY_KERN_DATA     2
#define X86_GDT_ENTRY_USER_CODE     3
#define X86_GDT_ENTRY_USER_DATA     4
#define X86_GDT_ENTRY_USER_TSS      5

#define X86_GDT_SEG_NULL            0
#define X86_GDT_SEG_KERN_CODE       8
#define X86_GDT_SEG_KERN_DATA       0x10
#define X86_GDT_SEG_USER_CODE       0x18
#define X86_GDT_SEG_USER_DATA       0x20
#define X86_GDT_SEG_USER_TSS        0x28

#define X86_GDT_RPL0                0
#define X86_GDT_RPL1                1
#define X86_GDT_RPL2                2
#define X86_GDT_RPL3                3

struct X86_GDT_Entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  limit_flags;
    uint8_t  base_high;
}__attribute__((packed));

struct X86_GDT_Descriptor {
    uint16_t  limit;
    uintptr_t base;
}__attribute__((packed));

struct X86_TSS {
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

extern void X86_GDT_Load(void* descriptor);
extern void X86_GDT_LoadTaskRegister(uint16_t selector);
extern void X86_GDT_LoadKernelSegments();
extern void X86_GDT_SetupEntry(struct X86_GDT_Entry* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

#endif
