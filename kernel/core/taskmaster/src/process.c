#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/scheduler.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/memory/include/physmem.h>
#include <kernel/clib/include/string.h>

struct SpinLock Process_lock;
struct Process* Process_primordial = PROCESS_NULL;
size_t Process_next_pid = 1;

// The process lock will be held from Schedule()
void Process_FirstEntryToUserSpace() {

	SpinLock_Release(&Process_lock);
}

bool Process_Initialize(struct Process* proc) {

	void* stack = Page_Acquire(KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
	if (stack == PAGE_LIST_NULL) return false;
	(proc->kernel_stack).base = stack;
	(proc->kernel_stack).size = KERNEL_STACK_SIZE;

	State_Initialize(proc);
	Context_Initialize(proc);

	if (!Paging_Initialize(proc)) return false;

	return true;
}

void Process_SleepOn(struct SleepLock* lock) {

    struct Process* proc = STATE_CURRENT->process;
    if (proc == PROCESS_NULL) return;
    if (STATE_CURRENT->preemption_vetos != 0) return;

    SpinLock_Acquire(&Process_lock);
    SpinLock_Release(&lock->access_lock);

    proc->wakeup_on = lock;
    proc->status = PROCESS_ASLEEP;
    SCHEDULER_RETURN;
    proc->wakeup_on = (void*)0;

    SpinLock_Release(&Process_lock);
    SpinLock_Acquire(&lock->access_lock);
}

void Process_Preempt() {

    struct Process* proc = STATE_CURRENT->process;

    if (proc == PROCESS_NULL) return;

    if (STATE_CURRENT->preemption_vetos != 0) return;

    SpinLock_Acquire(&Process_lock);

    if (Scheduler_Preempt(proc)) {
        proc->status = PROCESS_RUNNABLE;
        SCHEDULER_RETURN;
    }

    SpinLock_Release(&Process_lock);
}

uint32_t Process_Fork() {

	struct Process* parent_proc = STATE_CURRENT->process;
	struct Process* forked_proc = Scheduler_Book();

	if (!Process_Initialize(forked_proc) || !Paging_Clone(parent_proc, forked_proc)) {
		Page_Release((forked_proc->kernel_stack).base, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
		SpinLock_Acquire(&Process_lock);
		forked_proc->status = PROCESS_IDLE;
		SpinLock_Release(&Process_lock);
		return -1;
	}

	forked_proc->parent  = parent_proc;
	forked_proc->child   = PROCESS_NULL;
	forked_proc->sibling = parent_proc->child;
	forked_proc->num_children = 0;
	forked_proc->exit_code = -1;
	forked_proc->run_time = parent_proc->run_time;
	forked_proc->endpoint = parent_proc->endpoint;
	IContext_Fork(forked_proc->intr_context, parent_proc->intr_context);

	parent_proc->num_children++;
	parent_proc->child = forked_proc;

	SpinLock_Acquire(&Process_lock);
	forked_proc->status = PROCESS_RUNNABLE;
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
		if (proc->num_children == 0) {
			retval = -1;
			break;
		}

		struct Process* kid = proc->child;
		if (kid->status == PROCESS_DEAD) {
			retval = kid->id;
			kid->status = PROCESS_IDLE;
			proc->num_children--;
			proc->child = kid->sibling;
			break;
		}
		while (kid->sibling != PROCESS_NULL) {
			if (kid->sibling->status == PROCESS_DEAD) {
				retval = kid->sibling->id;
				kid->sibling->status = PROCESS_IDLE;
				proc->num_children--;
				kid->sibling = kid->sibling->sibling;
				break;
			}
			kid = kid->sibling;
		}

		proc->status = PROCESS_WAITING;
		SCHEDULER_RETURN;
	}
	
	SpinLock_Release(&Process_lock);
	
	return retval;
}

void Process_Exit(int status) {

    struct Process* proc = STATE_CURRENT->process;

    SpinLock_Acquire(&Process_lock);

    struct Process* kid = proc->child;
    while (kid != PROCESS_NULL) {
        kid->parent = Process_primordial;
        if (kid->status == PROCESS_DEAD && Process_primordial->status == PROCESS_WAITING) Process_primordial->status = PROCESS_RUNNABLE;
        kid = kid->sibling;
    }

    Paging_Terminate(proc);
    Page_Release((proc->kernel_stack).base, KERNEL_STACK_SIZE >> KERNEL_PAGE_SIZE_IN_BITS);
    proc->exit_code = status;
    proc->status = PROCESS_DEAD;

    if (proc->parent->status == PROCESS_WAITING) proc->parent->status = PROCESS_RUNNABLE;

	SCHEDULER_RETURN;
}

