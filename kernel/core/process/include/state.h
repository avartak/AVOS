#ifndef KERNEL_CORE_STATE_H
#define KERNEL_CORE_STATE_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#include <kernel/core/arch/include/arch.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/synch/include/spinlock.h>

struct State {
	size_t          preemption_vetos;
	uint8_t         interrupt_priority;
	struct Process* process;
	struct CPU*     cpu;
}__attribute__((packed));

#define STATE_CURRENT ((struct State*)(GetStackBase() - sizeof(struct State)))

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

extern size_t State_CPUBlockSize();

#endif
