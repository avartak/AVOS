#ifndef KERNEL_CORE_SYSCALL_H
#define KERNEL_CORE_SYSCALL_H

#include <kernel/core/arch/include/arch.h>

struct SysCall {

	uint32_t id;
	uint32_t flags;
	uint32_t (*handler)(struct IContext* frame);
};

extern struct SysCall SysCall_vector[];

extern void   SysCall(uint32_t id, struct IContext* frame);

#endif
