#include <kernel/core/setup/include/setup.h>
#include <kernel/core/syscall/include/syscall.h>
#include <kernel/core/taskmaster/include/state.h>

struct SysCall SysCall_vector[KERNEL_NUM_SYSCALLS];

void SysCall(uint32_t id, struct IContext* frame) {

	if (id >= KERNEL_NUM_SYSCALLS) return;
	SysCall_vector[id].handler(frame);
}

