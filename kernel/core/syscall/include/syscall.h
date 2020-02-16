#ifndef KERNEL_CORE_SYSCALL_H
#define KERNEL_CORE_SYSCALL_H

#include <kernel/core/arch/include/arch.h>

extern uint32_t (*SysCall_Handlers[])(void);

extern void SysCall_Initialize(uint8_t vector);
extern void SysCall(struct Interrupt_Frame* frame);

#endif
