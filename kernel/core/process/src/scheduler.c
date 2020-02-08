#include <kernel/core/process/include/scheduler.h>
#include <kernel/core/process/include/state.h>
#include <kernel/core/process/include/process.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/memory/include/virtmem.h>
#include <kernel/core/syscall/include/syscall.h>
#include <kernel/arch/i386/include/context.h>
#include <kernel/arch/i386/include/controlregs.h>
#include <kernel/arch/i386/include/interrupt.h>
#include <kernel/arch/initial/include/initialize.h>

struct Process Scheduler_processes[KERNEL_MAX_PROCS];

// Initialized the processes and related locks 
void Scheduler_Initialize() {

	SpinLock_Initialize(&Process_lock);
	for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) Scheduler_processes[i].life_cycle = PROCESS_IDLE;
	SpinLock_Initialize(&State_lock);

	SysCall_Initialize(0x80);
}

void Schedule() {

	size_t iproc = 0;
	while (true) {
		SpinLock_Acquire(&Process_lock);

		if (Scheduler_processes[iproc].life_cycle == PROCESS_KILLED  ) Scheduler_TerminateProcess(&Scheduler_processes[iproc]);
		if (Scheduler_processes[iproc].life_cycle == PROCESS_RUNNABLE) Scheduler_RunProcess(&Scheduler_processes[iproc]);
		iproc++;
		if (iproc == KERNEL_MAX_PROCS) iproc = 0;

		SpinLock_Release(&Process_lock);
	} 

}

// Should be run under the process lock
void Scheduler_RunProcess(struct Process* proc) {

	CPU_SetupProcess(proc);
	X86_CR3_Write((uintptr_t)Kernel_pagedirectory);

}

// Must run under the process lock and no other lock (we don't want other locks to be held while a process gets switched out)
void Scheduler_Return() {

	struct Process* proc = STATE_CURRENT->process;
	Context_Switch(&proc->context, STATE_CURRENT->cpu->scheduler);

}

bool Scheduler_ProcessShouldYield(struct Process* proc) {

	if (proc->life_cycle == PROCESS_KILLED && Interrupt_ReturningToUserMode(proc->interrupt_frame)) return true;
	else if (proc->life_cycle == PROCESS_RUNNING && STATE_CURRENT->cpu->timer_ticks > proc->start_time + proc->run_time) return true;
	else return false;
}

// Must run under the process lock
struct Process* Scheduler_Book() {

	for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
		if (Scheduler_processes[i].life_cycle == PROCESS_IDLE) {
			Scheduler_processes[i].life_cycle = PROCESS_BOOKED;
			Scheduler_processes[i].id = Process_next_pid++;
			return &Scheduler_processes[i];
		}
	}
	
	return (struct Process*)0;
}

// Must run under the process lock
void Scheduler_ChangeParent(struct Process* old_parent, struct Process* new_parent) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].parent == old_parent) {
			Scheduler_processes[i].parent = new_parent;
            if (Scheduler_processes[i].life_cycle == PROCESS_ZOMBIE) Scheduler_Wakeup(new_parent);
        }
    }

}

// Must run under the process lock
void Scheduler_Wakeup(struct Process* alarm) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].life_cycle == PROCESS_ASLEEP && Scheduler_processes[i].wakeup_on == alarm) Scheduler_processes[i].life_cycle = PROCESS_RUNNABLE;
    }

}

// Must run under the process lock
void Scheduler_WakeupFromSleepLock(struct SleepLock* lock) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].life_cycle == PROCESS_ASLEEP && Scheduler_processes[i].wakeup_on == lock) Scheduler_processes[i].life_cycle = PROCESS_RUNNABLE;
    }

}

// Must run under the process lock
bool Scheduler_ProcessHasKids(struct Process* proc) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].parent == proc) return true;
    }
	return false;

}

// Must run under the process lock
struct Process* Scheduler_GetProcessKid(struct Process* proc, enum Process_LifeCycle cycle) {
	
    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].parent == proc && Scheduler_processes[i].life_cycle == cycle) return &Scheduler_processes[i];
    }
	return (struct Process*)0;

}

// Must run under the process lock
void Scheduler_TerminateProcess(struct Process* proc) {

	if (proc->wakeup_on != proc) {
		struct SleepLock* sleep_lock = (struct SleepLock*)proc->wakeup_on;
		SleepLock_SilentRelease(sleep_lock);
		Scheduler_WakeupFromSleepLock(sleep_lock);
	}

    Scheduler_ChangeParent(proc, (struct Process*)0);
    proc->life_cycle = PROCESS_ZOMBIE;
    Memory_UnmakePageDirectory(proc->page_directory);
    Page_Release(proc->kernel_thread, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
    Scheduler_Wakeup(proc->parent);
}

// Must run under the process lock
void Scheduler_KillProcess(uint32_t process_id) {

    for (size_t i = 0; i < KERNEL_MAX_PROCS; i++) {
        if (Scheduler_processes[i].id == process_id && Scheduler_processes[i].life_cycle != PROCESS_IDLE && Scheduler_processes[i].life_cycle != PROCESS_ZOMBIE) {
			Scheduler_processes[i].life_cycle = PROCESS_KILLED;
		}
    }
}
