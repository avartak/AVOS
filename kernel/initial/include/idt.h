#ifndef KERNEL_INITIAL_IDT_H
#define KERNEL_INITIAL_IDT_H

#include <kernel/common/include/defs.h>

#define IDT_INT_TYPE_INTERRUPT  0xE
#define IDT_INT_TYPE_TRAP       0xF
#define IDT_INT_TYPE_TASK       0x5

#define IDT_INT_ACCESS_KERN     0x8
#define IDT_INT_ACCESS_USER     0xE

#define IDT_KERN_CODE_SEG       0x08

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

extern void IDT_Load(struct IDT_Descriptor* desc);

extern void IDT_SetupEntry(struct IDT_Entry* entry, uintptr_t address, uint16_t segment, uint8_t type);
extern void IDT_Initialize();

extern void Interrupt_0x0 ();
extern void Interrupt_0x1 ();
extern void Interrupt_0x2 ();
extern void Interrupt_0x3 ();
extern void Interrupt_0x4 ();
extern void Interrupt_0x5 ();
extern void Interrupt_0x6 ();
extern void Interrupt_0x7 ();
extern void Interrupt_0x8 ();
extern void Interrupt_0x9 ();
extern void Interrupt_0xA ();
extern void Interrupt_0xB ();
extern void Interrupt_0xC ();
extern void Interrupt_0xD ();
extern void Interrupt_0xE ();
extern void Interrupt_0xF ();
extern void Interrupt_0x10();
extern void Interrupt_0x11();
extern void Interrupt_0x12();
extern void Interrupt_0x13();
extern void Interrupt_0x14();
extern void Interrupt_0x15();
extern void Interrupt_0x16();
extern void Interrupt_0x17();
extern void Interrupt_0x18();
extern void Interrupt_0x19();
extern void Interrupt_0x1A();
extern void Interrupt_0x1B();
extern void Interrupt_0x1C();
extern void Interrupt_0x1D();
extern void Interrupt_0x1E();
extern void Interrupt_0x1F();

extern void Interrupt_0x20();
extern void Interrupt_0x21();
extern void Interrupt_0x22();
extern void Interrupt_0x23();
extern void Interrupt_0x24();
extern void Interrupt_0x25();
extern void Interrupt_0x26();
extern void Interrupt_0x27();
extern void Interrupt_0x28();
extern void Interrupt_0x29();
extern void Interrupt_0x2A();
extern void Interrupt_0x2B();
extern void Interrupt_0x2C();
extern void Interrupt_0x2D();
extern void Interrupt_0x2E();
extern void Interrupt_0x2F();

extern void Interrupt_0x80();

#endif
