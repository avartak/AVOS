#ifndef KERNEL_CORE_STATE_H
#define KERNEL_CORE_STATE_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

#include <kernel/arch/initial/include/kthread.h>
#include <kernel/arch/console/include/console.h>

struct State {

	size_t preemption_vetos;
	uint8_t interrupt_priority;

};

extern struct State* State_GetCurrent();

#define STATE_INCREMENT_PREEMPTION_VETO() \
	do { \
		State_GetCurrent()->preemption_vetos++; \
		atomic_signal_fence(memory_order_seq_cst); \
	} while (0)

#define STATE_DECREMENT_PREEMPTION_VETO() \
    do { \
		if (State_GetCurrent()->preemption_vetos == 0) Console_Panic("Panic(STATE_DECREMENT_PREEMPTION_VETO): No existing veto on preemption\n"); \
        atomic_signal_fence(memory_order_seq_cst); \
        State_GetCurrent()->preemption_vetos--; \
    } while (0)

#endif
