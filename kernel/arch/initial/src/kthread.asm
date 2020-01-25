%include "kernel/core/setup/include/setup.inc"

section .text

BITS 32

global KernelThread_GetStackBase
KernelThread_GetStackBase:
    mov  eax, esp
    and  eax, KERNEL_STACK_SIZE
	add  eax, KERNEL_STACK_SIZE

