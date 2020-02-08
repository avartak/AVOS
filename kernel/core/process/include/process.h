#ifndef KERNEL_CORE_PROCESS_H
#define KERNEL_CORE_PROCESS_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/core/process/include/context.h>
#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/synch/include/sleeplock.h>

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
	// Preemption
	uint64_t          start_time;
	size_t            run_time;
}__attribute__((packed));

enum Process_LifeCycle {PROCESS_IDLE, PROCESS_BOOKED, PROCESS_RUNNABLE, PROCESS_RUNNING, PROCESS_ASLEEP, PROCESS_KILLED, PROCESS_ZOMBIE};

extern struct SpinLock Process_lock;
extern size_t Process_next_pid;

extern bool     Process_Initialize(struct Process* proc);
extern void     Process_FirstEntryToUserSpace();
extern void     Process_Sleep();
extern void     Process_SleepOn(struct SleepLock* lock);
extern void     Process_Yield();

extern uint32_t Process_ID();
extern uint32_t Process_Fork();
extern void     Process_ChangeMemoryEndPoint(int32_t shift);
extern void     Process_Exit();
extern uint32_t Process_Wait();

#endif