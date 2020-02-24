#include <kernel/core/taskmaster/include/scheduler.h>
#include <kernel/core/taskmaster/include/dispatcher.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/syscall/include/syscall.h>

struct Process Scheduler_processes[KERNEL_MAX_PROCS];

void Scheduler_Initialize() {

	SpinLock_Initialize(&State_lock);
	SpinLock_Initialize(&Process_lock);
	SysCall_Initialize(0x80);

	for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) Scheduler_processes[i].life_cycle = PROCESS_IDLE;
}

void Schedule() {

	size_t iproc = 0;
	while (true) {
		SpinLock_Acquire(&Process_lock);
		if (Scheduler_processes[iproc].life_cycle == PROCESS_RUNNABLE) {
			Dispatch(&Scheduler_processes[iproc]);
		}
		SpinLock_Release(&Process_lock);

		iproc = (iproc == KERNEL_MAX_PROCS-1) ? 0 : iproc + 1;
	} 

}

// Must run under the process lock
bool Scheduler_Preempt(struct Process* proc) {

    if (proc->life_cycle == PROCESS_RUNNING && STATE_FROM_PROC(proc)->kernel_task->timer_ticks > proc->start_time + proc->run_time) return true;

	return false;
}

// Must run under the process lock
struct Process* Scheduler_Book() {

	while (true) {

		SpinLock_Acquire(&Process_lock);
		
		for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
			if (Scheduler_processes[i].life_cycle == PROCESS_IDLE) {
				Scheduler_processes[i].life_cycle = PROCESS_BOOKED;
				Scheduler_processes[i].id = Process_next_pid++;
				SpinLock_Release(&Process_lock);
				return &Scheduler_processes[i];
			}
		}
		
		SpinLock_Release(&Process_lock);

	}
	
}

// Must run under the process lock
void Scheduler_RaiseAlarm(void* alarm) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].life_cycle == PROCESS_ASLEEP && Scheduler_processes[i].wakeup_on == alarm) {
			Scheduler_processes[i].life_cycle = PROCESS_RUNNABLE;
			Scheduler_processes[i].wakeup_on  = (void*)0;
		}
    }
}

