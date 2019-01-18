#ifndef X86_KERNEL_SYSCALL_H
#define X86_KERNEL_SYSCALL_H

#include <x86/kernel/include/idt.h> 

#include <stdint.h>

extern inline void SysCall(uint32_t id);

#endif
