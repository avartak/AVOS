#ifndef KERNEL_CORE_STATE_H
#define KERNEL_CORE_STATE_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#include <kernel/core/arch/include/arch.h>
#include <kernel/core/synch/include/spinlock.h>

struct State {
	atomic_size_t   preemption_vetos;
	uint8_t         interrupt_priority;
	struct Process* process;
	struct KProc*   kernel_task;
}__attribute__((packed));

#define STATE_CURRENT           ((struct State*)(GetStackBase() - sizeof(struct State)))
#define STATE_FROM_PROC(proc)   ((struct State*)((uintptr_t)((proc->kernel_stack).base) + (proc->kernel_stack).size - sizeof(struct State)))

#define STATE_INCREMENT_PREEMPTION_VETO() \
    do { \
		atomic_fetch_add_explicit(&(STATE_CURRENT->preemption_vetos), 1, memory_order_relaxed); \
        atomic_signal_fence(memory_order_seq_cst); \
    } while (0)

#define STATE_DECREMENT_PREEMPTION_VETO() \
    do { \
        atomic_signal_fence(memory_order_seq_cst); \
		atomic_fetch_sub_explicit(&(STATE_CURRENT->preemption_vetos), 1, memory_order_relaxed); \
    } while (0)

#define STATE_PREEMPTIBLE() (STATE_CURRENT->preemption_vetos == 0)

extern struct SpinLock State_lock;

extern void State_Initialize(struct Process* proc);

#endif
