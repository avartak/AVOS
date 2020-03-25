#ifndef KERNEL_X86_CONTEXT_H
#define KERNEL_X86_CONTEXT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/taskmaster/include/stack.h>

struct KContext {
    uintptr_t* cr3;
    uint32_t   edi;
    uint32_t   esi;
    uint32_t   ebx;
    uint32_t   ebp;
    uintptr_t  eip;
}__attribute__((packed));

struct IContext {
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
}__attribute__((packed));


struct UContext {
	struct IContext  context;
	struct UContext* link;
	uint32_t         mask;
	struct Stack     stack;
}__attribute__((packed));


extern void   Context_Initialize(struct Process* proc);
extern void   Context_SetupProcess(struct Process* proc);

extern void   KContext_SetupProcess(struct Process* proc);
extern void   KContext_Switch(struct KContext** old_context, struct KContext* new_context);

extern void   IContext_Copy(struct IContext* dst, struct IContext* src);
extern void   IContext_Fork(struct IContext* dst, struct IContext* src);
extern bool   IContext_IsUserMode(struct IContext* frame);

#endif
