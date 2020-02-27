#ifndef KERNEL_CORE_ARCH_H
#define KERNEL_CORE_ARCH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct CPU;
struct KContext; 
struct IContext; 
struct Process;

extern uint32_t  Kernel_pagedirectory[];

extern void      Halt();
extern void      EnableInterrupts();
extern void      DisableInterrupts();
extern bool      InterruptsEnabled();
extern uintptr_t GetStackBase();

extern void      Context_Initialize(struct Process* proc);
extern void      Context_SetupProcess(struct Process* proc);

extern void      Interrupt_AddHandler(uint8_t entry, void (*handler)(struct IContext*));

extern void      KContext_Initialize(struct Process* proc);
extern void      KContext_SetupProcess(struct Process* proc);
extern void      KContext_Switch(struct KContext** old_context, struct KContext* new_context);

extern void      IContext_Initialize(struct Process* proc);
extern void      IContext_Copy(struct IContext* dst  , struct IContext* src);
extern void      IContext_Fork(struct IContext* child, struct IContext* parent);

extern void      Interrupt_AddHandler(uint8_t entry, void (*handler)(struct IContext*));

extern void      Console_Print(const char* format, ...);
extern void      Console_Panic(const char* format, ...);

extern size_t    Paging_MapPages(struct Process* proc, void* virt_addr, size_t size);
extern bool      Paging_UnmapPages(struct Process* proc, void* virt_addr, size_t size);
extern bool      Paging_Initialize(struct Process* proc);
extern void      Paging_Terminate(struct Process* proc);
extern bool      Paging_Clone(struct Process* proc, struct Process* clone_proc);

#endif
