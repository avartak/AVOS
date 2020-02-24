#ifndef KERNEL_X86_CONTEXT_H
#define KERNEL_X86_CONTEXT_H

#include <stdint.h>
#include <stddef.h>

struct Process;

struct Context {
	uintptr_t* cr3;
	uint32_t   edi;
	uint32_t   esi;
	uint32_t   ebx;
	uint32_t   ebp;
	uintptr_t  eip;
}__attribute__((packed));

extern void   Context_Initialize(struct Process* proc);
extern void   Context_SetupProcess(struct Process* proc);
extern void   Context_Switch(struct Context** old_context, struct Context* new_context);

#endif
