#ifndef KERNEL_CORE_PROCESS_H
#define KERNEL_CORE_PROCESS_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/core/setup/include/setup.h>
#include <kernel/core/arch/include/arch.h>
#include <kernel/core/taskmaster/include/stack.h>
#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/synch/include/sleeplock.h>
#include <kernel/core/signal/include/signal.h>

#define PROCESS_NULL  (struct Process*)0

#define PROCESS_ID()  (STATE_CURRENT->process->id)

enum Process_Status {PROCESS_IDLE, PROCESS_BOOKED, PROCESS_RUNNABLE, PROCESS_RUNNING, PROCESS_ASLEEP, PROCESS_WAITING, PROCESS_DEAD};

struct Process {
	// Provenance
	pid_t                   id;
	uint32_t                user;
	struct Process*         parent;
	struct Process*         child;
	struct Process*         sibling;
	size_t                  num_children;
	// Status
	enum Process_Status     status;
	int                     exit_code;
	// Memory
	uintptr_t               endpoint;
	// Execution
	struct Stack            kernel_stack;
	struct KContext*        task_context;
	struct IContext*        intr_context;
	void*                   wakeup_on;
	// Signal
	struct Stack            signal_stack;
	sigset_t                signal_bitmap;
	sigset_t                signal_bitmask;
	struct Signal           signal_queue[KERNEL_NUM_SIGNALS];
	// Preemption
	uint64_t                start_time;
	size_t                  run_time;
}__attribute__((packed));

struct KProc {
	struct CPU* cpu;
    struct KContext* scheduler;
    uint64_t timer_ticks;
}__attribute__((packed));

extern struct SpinLock Process_lock;
extern struct Process* Process_primordial;
extern size_t          Process_next_pid;

extern bool            Process_Initialize(struct Process* proc);
extern void            Process_FirstEntryToUserSpace();
extern void            Process_SleepOn(struct SleepLock* lock);
extern void            Process_Preempt();

extern pid_t           Process_Fork();
extern void            Process_ShiftEndPoint(int32_t shift);
extern void            Process_Exit(int status);
extern pid_t           Process_Wait();

#endif
