#ifndef X86_KERNEL_IDT_H
#define X86_KERNEL_IDT_H

#include <stdint.h>

#define IDT_INT_TYPE_INTERRUPT  0xE
#define IDT_INT_TYPE_TRAP       0xF
#define IDT_INT_TYPE_TASK       0x5

#define IDT_INT_ACCESS_KERN     0x8
#define IDT_INT_ACCESS_USER     0xE

#define IDT_KERN_CODE_SEG       0x08

#define asm __asm__
#define volatile __volatile__

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

extern struct IDT_Entry      IDT_entries[];
extern struct IDT_Descriptor IDT_desc;

extern void IDT_SetupEntry(struct IDT_Entry* entry, uintptr_t address, uint16_t segment, uint8_t type);
extern void IDT_Initialize();

static inline void IDT_Load(struct IDT_Descriptor* gdtr);

inline void IDT_Load(struct IDT_Descriptor* desc) {
    asm volatile("lidt %0" : : "m"(*desc));
}

#endif
