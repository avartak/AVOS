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

extern void Interrupt_0 ();
extern void Interrupt_1 ();
extern void Interrupt_2 ();
extern void Interrupt_3 ();
extern void Interrupt_4 ();
extern void Interrupt_5 ();
extern void Interrupt_6 ();
extern void Interrupt_7 ();
extern void Interrupt_8 ();
extern void Interrupt_9 ();
extern void Interrupt_A ();
extern void Interrupt_B ();
extern void Interrupt_C ();
extern void Interrupt_D ();
extern void Interrupt_E ();
extern void Interrupt_F ();
extern void Interrupt_10();
extern void Interrupt_11();
extern void Interrupt_12();
extern void Interrupt_13();
extern void Interrupt_14();
extern void Interrupt_15();
extern void Interrupt_16();
extern void Interrupt_17();
extern void Interrupt_18();
extern void Interrupt_19();
extern void Interrupt_1A();
extern void Interrupt_1B();
extern void Interrupt_1C();
extern void Interrupt_1D();
extern void Interrupt_1E();
extern void Interrupt_1F();

extern void Interrupt_20();
extern void Interrupt_21();
extern void Interrupt_22();
extern void Interrupt_23();
extern void Interrupt_24();
extern void Interrupt_25();
extern void Interrupt_26();
extern void Interrupt_27();
extern void Interrupt_28();
extern void Interrupt_29();
extern void Interrupt_2A();
extern void Interrupt_2B();
extern void Interrupt_2C();
extern void Interrupt_2D();
extern void Interrupt_2E();
extern void Interrupt_2F();

extern void Interrupt_80();

#endif
