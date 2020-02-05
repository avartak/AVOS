#ifndef KERNEL_CORE_IRQLOCK_H
#define KERNEL_CORE_IRQLOCK_H

#include <kernel/core/synch/include/spinlock.h>

#include <stdint.h>

struct IRQLock {
	struct SpinLock lock;
	uint8_t previous_interrupt_priority;
};

extern void IRQLock_Initialize(struct IRQLock* lock);
extern void IRQLock_Acquire(struct IRQLock* lock);
extern void IRQLock_Release(struct IRQLock* lock);

#endif
