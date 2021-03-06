#ifndef KERNEL_CORE_SLEEPLOCK_H
#define KERNEL_CORE_SLEEPLOCK_H

#include <kernel/core/synch/include/spinlock.h>

#include <stdint.h>

struct SleepLock {
	struct SpinLock access_lock;
	bool locked;
};

extern void SleepLock_Initialize(struct SleepLock* lock);
extern void SleepLock_Acquire(struct SleepLock* lock);
extern void SleepLock_Release(struct SleepLock* lock);

#endif
