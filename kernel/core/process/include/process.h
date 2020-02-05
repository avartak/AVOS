#ifndef KERNEL_CORE_PROCESS_H
#define KERNEL_CORE_PROCESS_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/core/process/include/context.h>
#include <kernel/core/synch/include/spinlock.h>

struct Process {
	// Provenance
    uint32_t          id;
    struct Process*   parent;
	// Status
    uint8_t           life_cycle;
	int               exit_status;
	// Memory
    uintptr_t*        page_directory;
	uintptr_t         memory_endpoint;
	// Execution
    struct Context*   context;
    uint8_t*          kernel_thread;
    struct Interrupt_Frame* interrupt_frame;
	// Signal
    void*             wakeup_on;
}__attribute__((packed));

enum Process_LifeCycle {PROCESS_IDLE, PROCESS_BOOKED, PROCESS_RUNNABLE, PROCESS_RUNNING, PROCESS_ASLEEP, PROCESS_ZOMBIE};

extern struct SpinLock Process_lock;
extern size_t Process_next_pid;

extern bool     Process_Initialize(struct Process* proc);
extern void     Process_ChangeMemoryEndPoint(struct Process* proc, int32_t shift);
extern void     Process_PrepareSwitch(struct Process* proc);
extern void     Process_FirstEntryToUserSpace();

extern uint32_t Process_Fork();
extern void     Process_Exit();

#endif
