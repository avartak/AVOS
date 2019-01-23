#ifndef X86_KERNEL_STATE_H
#define X86_KERNEL_STATE_H

#include <stdint.h>

struct State {

	uint32_t  privilege;
	uint16_t  ss0;
	uint32_t  esp0;
	uint32_t  cr3;
	uint32_t  eip;
	uint32_t  eflags;
	uint32_t  eax;
	uint32_t  ecx;
	uint32_t  edx;
	uint32_t  ebx;
	uint32_t  esp;
	uint32_t  ebp;
	uint32_t  esi;
	uint32_t  edi;
	uint16_t  es;
	uint16_t  cs;
	uint16_t  ss;
	uint16_t  ds;
	uint16_t  fs;
	uint16_t  gs;
	uint16_t  ldt;
	uintptr_t iomap;	

}__attribute__((packed));

extern struct    State State_current;
extern uintptr_t State_current_ptr;

#endif
