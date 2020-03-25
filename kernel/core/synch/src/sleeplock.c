#include <kernel/core/synch/include/sleeplock.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/taskmaster/include/scheduler.h>

void SleepLock_Initialize(struct SleepLock* lock) {
	SpinLock_Initialize(&(lock->access_lock));
	lock->locked = false;	
}

void SleepLock_Acquire(struct SleepLock* lock) {

	struct Process* proc = STATE_CURRENT->process;
	if (proc == PROCESS_NULL) return;

	SpinLock_Acquire(&(lock->access_lock));
	while (lock->locked) {
		if (proc != PROCESS_NULL) {
    		SpinLock_Release(&lock->access_lock);
    		SpinLock_Acquire(&Process_lock);
			Process_SleepOn(lock);
    		SpinLock_Release(&Process_lock);
    		SpinLock_Acquire(&lock->access_lock);
		}
	}
	lock->locked = true;
	SpinLock_Release(&(lock->access_lock));
}

// Process lock should not be held when running this function
void SleepLock_Release(struct SleepLock* lock) {

	SpinLock_Acquire(&(lock->access_lock));
	lock->locked = false;

	SpinLock_Acquire(&Process_lock);
	Scheduler_RaiseAlarm(lock);
	SpinLock_Release(&Process_lock);

	SpinLock_Release(&(lock->access_lock));
}

