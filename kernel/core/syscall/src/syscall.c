#include <kernel/core/setup/include/setup.h>
#include <kernel/core/syscall/include/syscall.h>
#include <kernel/core/process/include/state.h>

uint32_t (*SysCall_Handlers[KERNEL_NUM_SYSCALLS])(void) = {};

void SysCall_Initialize(uint8_t vector) {
    Interrupt_AddEntry(vector, SysCall);
}

void SysCall(__attribute__((unused))struct Interrupt_Frame* frame) {

	uint32_t isys = frame->eax;
	if (isys >= KERNEL_NUM_SYSCALLS) return;

	struct Process* proc = STATE_CURRENT->process;
	if (proc != (struct Process*)0 && proc->life_cycle != PROCESS_RUNNING) return;

	SysCall_Handlers[isys]();
}
