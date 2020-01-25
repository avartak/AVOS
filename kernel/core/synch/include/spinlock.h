#ifndef KERNEL_CORE_SPINLOCK_H
#define KERNEL_CORE_SPINLOCK_H

#include <stdbool.h>
#include <stdatomic.h>

struct SpinLock {
	atomic_size_t flag;
	const char* name;
};

extern void SpinLock_Initialize(struct SpinLock* lock, const char* name);
extern void SpinLock_Acquire(struct SpinLock* lock);
extern void SpinLock_Release(struct SpinLock* lock);
extern bool SpinLock_Locked(struct SpinLock* lock);

#endif
