#ifndef KERNEL_X86_CONTEXT_H
#define KERNEL_X86_CONTEXT_H

#include <stdint.h>
#include <stddef.h>

struct Context {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
}__attribute__((packed));

extern size_t Context_GetStructSize();
extern void   Context_Switch(struct Context** old_context, struct Context* new_context);
extern void   Context_SetProgramCounter(struct Context* context, uintptr_t ptr);

#endif
