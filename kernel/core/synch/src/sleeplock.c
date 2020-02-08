#include <kernel/core/synch/include/sleeplock.h>
#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/process/include/scheduler.h>

void SleepLock_Initialize(struct SleepLock* lock) {
	SpinLock_Initialize(&(lock->access_lock));
	lock->locked = false;	
	lock->locked_process = 0;
}

void SleepLock_Acquire(struct SleepLock* lock) {

	struct Process* proc = STATE_CURRENT->process;
	if (proc == (struct Process*)0) return;

	SpinLock_Acquire(&(lock->access_lock));
	while (lock->locked) Process_SleepOn(lock);
	lock->locked = true;
	lock->locked_process = proc->id;
	SpinLock_Release(&(lock->access_lock));
}

// Process lock should not be held when running this function
void SleepLock_Release(struct SleepLock* lock) {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;

	SpinLock_Acquire(&(lock->access_lock));
	lock->locked = false;
	lock->locked_process = 0;

	SpinLock_Acquire(&Process_lock);
	Scheduler_WakeupFromSleepLock(lock);
	SpinLock_Release(&Process_lock);

	SpinLock_Release(&(lock->access_lock));
}

// Process lock can be held when running this function
void SleepLock_SilentRelease(struct SleepLock* lock) {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;

    SpinLock_Acquire(&(lock->access_lock));
    lock->locked = false;
    lock->locked_process = 0;

    SpinLock_Release(&(lock->access_lock));
}

