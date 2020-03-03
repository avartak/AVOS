#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/taskmaster/include/state.h>

void SpinLock_Initialize(struct SpinLock* lock) {
    atomic_flag_clear_explicit(&(lock->flag), memory_order_relaxed);
}

void SpinLock_Acquire(struct SpinLock* lock) {
    STATE_INCREMENT_PREEMPTION_VETO();
    while (atomic_flag_test_and_set_explicit(&(lock->flag), memory_order_acquire)) {
		__asm__("pause");
	}
}

void SpinLock_Release(struct SpinLock* lock) {
    atomic_flag_clear_explicit(&lock->flag, memory_order_release);
    STATE_DECREMENT_PREEMPTION_VETO();
}

bool SpinLock_Locked(struct SpinLock* lock) {
	bool flag = true;
	atomic_compare_exchange_strong(&lock->flag, &flag, flag);
	return flag;
}
