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

#define KERNEL_CODE_SEG     0x08

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
extern struct IDTRecord idtr;

extern void SetupIDTEntry(struct IDTEntry* entry, uintptr_t address, uint16_t segment, uint8_t type);
extern void SetupIDT();

extern inline void LoadIDT(struct IDTRecord* gdtr);


struct StackStateAtInterrupt {
	uint32_t eflags;
	uint32_t cs;
	uint32_t eip;
	uint32_t error_code;
}__attribute__((packed));

struct CPUStateAtInterrupt {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
}__attribute__((packed));

extern void HandleInterrupt(struct CPUStateAtInterrupt cpu, struct StackStateAtInterrupt stack, uint32_t interrupt);

extern void Interrupt0 ();
extern void Interrupt1 ();
extern void Interrupt2 ();
extern void Interrupt3 ();
extern void Interrupt4 ();
extern void Interrupt5 ();
extern void Interrupt6 ();
extern void Interrupt7 ();
extern void Interrupt8 ();
extern void Interrupt9 ();
extern void InterruptA ();
extern void InterruptB ();
extern void InterruptC ();
extern void InterruptD ();
extern void InterruptE ();
extern void InterruptF ();
extern void Interrupt10();
extern void Interrupt11();
extern void Interrupt12();
extern void Interrupt13();
extern void Interrupt14();
extern void Interrupt15();
extern void Interrupt16();
extern void Interrupt17();
extern void Interrupt18();
extern void Interrupt19();
extern void Interrupt1A();
extern void Interrupt1B();
extern void Interrupt1C();
extern void Interrupt1D();
extern void Interrupt1E();
extern void Interrupt1F();

extern void Interrupt20();
extern void Interrupt21();
extern void Interrupt22();
extern void Interrupt23();
extern void Interrupt24();
extern void Interrupt25();
extern void Interrupt26();
extern void Interrupt27();
extern void Interrupt28();
extern void Interrupt29();
extern void Interrupt2A();
extern void Interrupt2B();
extern void Interrupt2C();
extern void Interrupt2D();
extern void Interrupt2E();
extern void Interrupt2F();

extern void Interrupt80();

#endif
