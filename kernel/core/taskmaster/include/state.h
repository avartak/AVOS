#ifndef KERNEL_CORE_STATE_H
#define KERNEL_CORE_STATE_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#include <kernel/core/arch/include/arch.h>
#include <kernel/core/synch/include/spinlock.h>

struct State {
	size_t          preemption_vetos;
	uint8_t         interrupt_priority;
	struct Process* process;
	struct KProc*   kernel_task;
}__attribute__((packed));

#define STATE_CURRENT           ((struct State*)(GetStackBase() - sizeof(struct State)))
#define STATE_FROM_PROC(proc)   ((struct State*)((uintptr_t)((proc->kernel_stack).base) + (proc->kernel_stack).size - sizeof(struct State)))

#define STATE_INCREMENT_PREEMPTION_VETO() \
    do { \
        STATE_CURRENT->preemption_vetos++; \
        atomic_signal_fence(memory_order_seq_cst); \
    } while (0)

#define STATE_DECREMENT_PREEMPTION_VETO() \
    do { \
        if (STATE_CURRENT->preemption_vetos == 0) Console_Panic("Panic(STATE_DECREMENT_PREEMPTION_VETO): No existing veto on preemption\n"); \
        atomic_signal_fence(memory_order_seq_cst); \
        STATE_CURRENT->preemption_vetos--; \
    } while (0)

extern struct SpinLock State_lock;

extern void State_Initialize(struct Process* proc);

#endif
