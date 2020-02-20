#include <kernel/arch/tasking/include/context.h>
#include <kernel/arch/tasking/include/cpu.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/process.h>

size_t Context_GetStructSize() {

	return sizeof(struct Context);
}

void Context_SetupProcess(struct Process* proc) {

    struct State* proc_state = (struct State*)(proc->kstack + KERNEL_STACK_SIZE - sizeof(struct State));
    proc_state->kernel_task = STATE_CURRENT->kernel_task;

    STATE_CURRENT->kernel_task->cpu->task_state.esp0 = (uintptr_t)proc_state;
    proc->start_time = STATE_CURRENT->kernel_task->timer_ticks;
}

void Context_SetProgramCounter(struct Context* context, uintptr_t  ptr) {

	context->eip = ptr;
}

