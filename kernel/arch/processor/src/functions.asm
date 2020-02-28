%include "kernel/core/setup/include/setup.inc"

section .text

global Halt
Halt:
    hlt
    ret

global DisableInterrupts
DisableInterrupts:
    cli
    ret

global EnableInterrupts
EnableInterrupts:
    sti
    ret

global GetStackBase
GetStackBase:
	mov  eax, esp
	and  eax, ~(KERNEL_STACK_SIZE-1)
	add  eax, KERNEL_STACK_SIZE
	ret
