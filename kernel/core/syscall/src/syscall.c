#include <kernel/core/setup/include/setup.h>
#include <kernel/core/syscall/include/syscall.h>
#include <kernel/core/taskmaster/include/state.h>

uint32_t (*SysCall_Handlers[KERNEL_NUM_SYSCALLS])(void) = {};

void SysCall_Initialize(uint8_t vector) {
    Interrupt_AddHandler(vector, SysCall);
}

void SysCall(struct Interrupt_Frame* frame) {

	uint32_t isys = Interrupt_GetReturnRegister(frame);
	if (isys >= KERNEL_NUM_SYSCALLS) return;

	struct Process* proc = STATE_CURRENT->process;
	if (proc == (struct Process*)0) return;
	if (proc->signaled_change == PROCESS_KILLED) return;
	if (proc->signaled_change == PROCESS_ASLEEP) return;

	SysCall_Handlers[isys]();
}
