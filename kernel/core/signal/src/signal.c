#include <kernel/core/signal/include/signal.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/taskmaster/include/scheduler.h>

bool Signal_Transmit(__attribute__((unused)) pid_t proc_id) {

    SpinLock_Acquire(&Process_lock);

	// Add signal to the process queue here

	SpinLock_Release(&Process_lock);

	return true;

}

