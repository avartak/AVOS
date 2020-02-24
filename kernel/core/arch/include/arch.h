#ifndef KERNEL_CORE_ARCH_H
#define KERNEL_CORE_ARCH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct CPU;
struct Context; 
struct Interrupt_Frame;
struct Process;

extern uint32_t        Kernel_pagedirectory[];
extern struct CPU*     Kernel_cpus[];
extern struct Process* Kernel_process_primordial;

extern void            Halt();
extern void            EnableInterrupts();
extern void            DisableInterrupts();
extern bool            InterruptsEnabled();
extern uintptr_t       GetStackBase();

extern void            Context_Initialize(struct Process* proc);
extern void            Context_SetupProcess(struct Process* proc);
extern void            Context_Switch(struct Context** old_context, struct Context* new_context);

extern void            Interrupt_Frame_Initialize(struct Process* proc);
extern void            Interrupt_AddHandler(uint8_t entry, void (*handler)(struct Interrupt_Frame*));
extern size_t          Interrupt_GetReturnRegister(struct Interrupt_Frame* frame);
extern size_t          Interrupt_GetVector(struct Interrupt_Frame* frame);
extern void            Interrupt_Frame_Fork(struct Interrupt_Frame* dst, struct Interrupt_Frame* src);

extern void            Console_Print(const char* format, ...);
extern void            Console_Panic(const char* format, ...);

extern bool            Paging_IsPageMapped(struct Process* proc, uintptr_t virt_addr);
extern bool            Paging_MapPage(struct Process* proc, uintptr_t virt_addr, uintptr_t phys_addr, bool create);
extern bool            Paging_UnmapPage(struct Process* proc, uintptr_t virt_addr);
extern size_t          Paging_MapPages(struct Process* proc, void* virt_addr, size_t size);
extern bool            Paging_UnmapPages(struct Process* proc, void* virt_addr, size_t size);
extern bool            Paging_Initialize(struct Process* proc);
extern void            Paging_Terminate(struct Process* proc);
extern bool            Paging_Clone(struct Process* proc, struct Process* clone_proc);
extern uintptr_t       Paging_GetHigherHalfAddress(struct Process* proc, uintptr_t user_addr);
extern bool            Paging_Copy(struct Process* proc, uintptr_t dest_addr, uintptr_t src_addr, size_t nbytes);

#endif
