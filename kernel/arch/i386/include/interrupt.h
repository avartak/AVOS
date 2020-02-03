#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

#include <stdint.h>

#include <kernel/core/process/include/state.h>

struct Interrupt_Frame {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t oesp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	
	uint16_t gs;
	uint16_t padding1;
	uint16_t fs;
	uint16_t padding2;
	uint16_t es;
	uint16_t padding3;
	uint16_t ds;
	uint16_t padding4;

	uint32_t vector;
	
	uint32_t err;
	uint32_t eip;
	uint16_t cs;
	uint16_t padding5;
	uint32_t eflags;
	
	uint32_t esp;
	uint16_t ss;
	uint16_t padding6;
};

extern void Interrupt_0x20();
extern void Interrupt_0x21();
extern void Interrupt_0x30();

extern void (*Interrupt_Handlers[])(struct Interrupt_Frame*);

extern void Interrupt_Return();
extern void Interrupt_BaseHandler(struct Interrupt_Frame* frame);
extern void Interrupt_AddEntry(uint8_t entry, void (*handler)(struct Interrupt_Frame*));

#endif
