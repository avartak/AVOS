#include <kernel/core/setup/include/setup.h>
#include <kernel/core/syscall/include/syscall.h>
#include <kernel/core/taskmaster/include/state.h>

uint32_t (*SysCall_Handlers[KERNEL_NUM_SYSCALLS])(void) = {};

void SysCall_Initialize(uint8_t vector) {
    Interrupt_AddHandler(vector, SysCall);
}

void SysCall(struct Interrupt_Frame* frame) {

	SysCall_Handlers[Interrupt_GetReturnRegister(frame)]();
}
