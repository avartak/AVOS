#ifndef KERNEL_CORE_ARCH_H
#define KERNEL_CORE_ARCH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct CPU;
struct Context; 
struct Interrupt_Frame;
struct Process;

extern size_t Context_GetStructSize();
extern size_t CPU_GetStructSize();
extern size_t Interrupt_Frame_GetStructSize();

extern size_t      Kernel_numcpus_online; 
extern uint32_t    Kernel_pagedirectory[];
extern struct CPU* Kernel_cpus[];

extern void      Halt();
extern void      EnableInterrupts();
extern void      DisableInterrupts();
extern bool      InterruptsEnabled();
extern uintptr_t GetStackBase();

extern void     CPU_SetupProcess(struct Process* proc);
extern void     CPU_CleanupProcess(struct Process* proc);
extern uint64_t CPU_GetTimerTicks(struct CPU* cpu);
extern struct Context*  CPU_GetScheduler(struct CPU* cpu);
extern struct Context** CPU_GetSchedulerPtr(struct CPU* cpu);

extern void   Context_Switch(struct Context** old_context, struct Context* new_context);
extern void   Context_SetProgramCounter(struct Context* context, uintptr_t ptr);

extern void   Interrupt_Return();
extern void   Interrupt_BaseHandler(struct Interrupt_Frame* frame);
extern void   Interrupt_AddEntry(uint8_t entry, void (*handler)(struct Interrupt_Frame*));
extern size_t Interrupt_GetReturnRegister(struct Interrupt_Frame* frame);
extern void   Interrupt_SetReturnRegister(struct Interrupt_Frame* frame, size_t value);
extern size_t Interrupt_GetVector(struct Interrupt_Frame* frame);
extern bool   Interrupt_ReturningToUserMode(struct Interrupt_Frame* frame);
extern void   Interrupt_CopyFrame(struct Interrupt_Frame* dst, struct Interrupt_Frame* src);

extern void Console_Print(const char* format, ...);
extern void Console_Panic(const char* format, ...);

extern bool      Paging_IsPageMapped(uintptr_t* page_dir, uintptr_t virt_addr);
extern bool      Paging_MapPage(uintptr_t* page_dir, uintptr_t virt_addr, uintptr_t phys_addr, bool create);
extern bool      Paging_UnmapPage(uintptr_t* page_dir, uintptr_t virt_addr);
extern size_t    Paging_MapPages(uintptr_t* page_dir, void* virt_addr, size_t size);
extern bool      Paging_UnmapPages(uintptr_t* page_dir, void* virt_addr, size_t size);
extern bool      Paging_MakePageDirectory(uintptr_t* page_dir);
extern void      Paging_UnmakePageDirectory(uintptr_t* page_dir);
extern bool      Paging_ClonePageDirectory(uintptr_t* page_dir, uintptr_t* clone);
extern uintptr_t Paging_GetHigherHalfAddress(uintptr_t* page_dir, uintptr_t user_addr);
extern bool      Paging_Copy(uintptr_t* dest_page_dir, uintptr_t dest_addr, uintptr_t src_addr, size_t nbytes);

#endif
