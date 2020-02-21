#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/scheduler.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/memory/include/physmem.h>
#include <kernel/clib/include/string.h>

struct SpinLock Process_lock;
size_t Process_next_pid = 1;

// The process lock will be held from Schedule()
void Process_FirstEntryToUserSpace() {

	SpinLock_Release(&Process_lock);
}

bool Process_Initialize(struct Process* proc) {

	// Allocate stack for the kernel thread
	void* kstack = Page_Acquire(KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
	if (kstack == PAGE_LIST_NULL) return false;

	// Now we go about initializing the process
	proc->kstack = (uint8_t*)kstack;
	uintptr_t stack_ptr = (uintptr_t)proc->kstack + KERNEL_STACK_SIZE;
	
	// State goes at the very base of the stack 
	stack_ptr -= sizeof(struct State);
	((struct State*)stack_ptr)->process = proc;
	((struct State*)stack_ptr)->preemption_vetos = 0;
	((struct State*)stack_ptr)->interrupt_priority = 0;

	// Next comes the interrupt frame
	stack_ptr -= Interrupt_Frame_GetStructSize();
	proc->interrupt_frame = (struct Interrupt_Frame*)stack_ptr;
	
	// Then, there is space for the pointer to the interrupt frame (which gets passed to the interrupt handler)
	// Optionally it can also hold the address of the part of the interrupt handling code that handles return from the interrupt (to enter user space)
	stack_ptr -= sizeof(uintptr_t);
	*((uintptr_t*)stack_ptr) = (uintptr_t)Interrupt_Return;

	// The process context comes next
	size_t context_size = Context_GetStructSize();
	stack_ptr -= context_size;
	proc->context = (struct Context*)stack_ptr;
	memset(proc->context, 0, context_size);
	Context_SetProgramCounter(proc->context, (uintptr_t)Process_FirstEntryToUserSpace);

	// Setup process memory
	if (!Paging_MakePageDirectory(proc)) {
		Page_Release(kstack, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
		return false;
	}

	return true;
}

void Process_SleepOn(struct SleepLock* lock) {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == (struct Process*)0) return;
    if (STATE_CURRENT->preemption_vetos != 0) return;

    SpinLock_Acquire(&Process_lock);
    SpinLock_Release(&lock->access_lock);

    proc->wakeup_on = lock;
    proc->life_cycle = PROCESS_ASLEEP;
    SCHEDULER_RETURN;
    proc->wakeup_on = (void*)0;

    SpinLock_Release(&Process_lock);
    SpinLock_Acquire(&lock->access_lock);
}

uint32_t Process_ID() {

	return STATE_CURRENT->process->id;
}

uint32_t Process_Fork() {

	struct Process* parent_proc = STATE_CURRENT->process;
	struct Process* forked_proc = Scheduler_Book();

	bool release_proc = false;
	if (!Process_Initialize(forked_proc)) release_proc = true;
	if (!Paging_ClonePageDirectory(parent_proc, forked_proc)) {
		Page_Release(forked_proc->kstack, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
		release_proc = true;
	}
	if (release_proc) {
		SpinLock_Acquire(&Process_lock);
		forked_proc->life_cycle = PROCESS_IDLE;
		SpinLock_Release(&Process_lock);
		return -1;
	}

	forked_proc->parent = parent_proc;
	forked_proc->exit_status = -1;
	forked_proc->run_time = parent_proc->run_time;
	forked_proc->endpoint = parent_proc->endpoint;
	Interrupt_CopyFrame(forked_proc->interrupt_frame, parent_proc->interrupt_frame);
	Interrupt_SetReturnRegister(forked_proc->interrupt_frame, 0);

	SpinLock_Acquire(&Process_lock);
	forked_proc->life_cycle = PROCESS_RUNNABLE;
	SpinLock_Release(&Process_lock);

	return STATE_CURRENT->process->id;

}

void Process_ShiftEndPoint(int32_t shift) {

	struct Process* proc = STATE_CURRENT->process;
	
	uintptr_t old_end = proc->endpoint;
	uintptr_t new_end = (int32_t)(proc->endpoint) + shift;
	
	if (shift == 0) return;
	else if (shift < 0) Paging_UnmapPages(proc, (void*)new_end, -shift);
	else {
	    size_t size_alloc = Paging_MapPages(proc, (void*)old_end, shift);
	    uintptr_t old_page = old_end & ~(KERNEL_PAGE_SIZE_IN_BYTES-1);
	    if (old_page + size_alloc < new_end) new_end = old_page + size_alloc;
	}
	
	proc->endpoint = new_end;
}

// No locks should be held by the process when waiting
uint32_t Process_Wait() {

	SpinLock_Acquire(&Process_lock);
	
	uint32_t retval = -1;
	struct Process* proc = STATE_CURRENT->process;
	while (true) {
		struct Process* zombie_kid = Scheduler_GetProcessKid(proc, PROCESS_DEAD);
		if (zombie_kid == (struct Process*)(-1)) {
			retval = -1;
			break;
		}
		if (zombie_kid != (struct Process*)0) {
			retval = zombie_kid->id;
			zombie_kid->life_cycle = PROCESS_IDLE;
			break;
		}
		proc->life_cycle = PROCESS_WAITING;
		SCHEDULER_RETURN;
	}
	
	SpinLock_Release(&Process_lock);
	
	return retval;
}

void Process_Exit(int status) {

    struct Process* proc = STATE_CURRENT->process;

    SpinLock_Acquire(&Process_lock);

	Scheduler_TerminateProcess(proc, status);

	SCHEDULER_RETURN;
}
