#ifndef X86_KERNEL_IDT_H
#define X86_KERNEL_IDT_H

#include <stdint.h>

#define INT_TYPE_INTERRUPT  0xE
#define INT_TYPE_TRAP       0xF
#define INT_TYPE_TASK       0x5

#define INT_ACCESS_KERNEL   0x8
#define INT_ACCESS_USER1    0xA
#define INT_ACCESS_USER2    0xC
#define INT_ACCESS_USER3    0xE

struct IDTEntry{
	uint16_t addr_low;
	uint16_t segment;
	uint8_t  zero;
	uint8_t  type_attr;
	uint16_t addr_high;
}__attribute__((packed));
 
struct IDTRecord {
    uint16_t  limit;
    uintptr_t base;
}__attribute__((packed));

extern struct IDTEntry idt[];
extern struct IDTRecord gdtr;

extern void SetupIDTEntry(struct IDTEntry* entry, uint32_t address, uint16_t segment, uint8_t type);
extern void SetupIDT();

extern inline void LoadIDT(struct IDTRecord* gdtr);

#endif
