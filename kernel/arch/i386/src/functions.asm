%include "kernel/core/setup/include/setup.inc"

section .text

global X86_Halt
X86_Halt:
	hlt
	ret

global X86_DisableInterrupts
X86_DisableInterrupts:
	cli
	ret

global X86_EnableInterrupts
X86_EnableInterrupts:
	sti
	ret

global X86_GetStackBase
X86_GetStackBase:
	mov  eax, esp
	and  eax, ~(KERNEL_STACK_SIZE-1)
	add  eax, KERNEL_STACK_SIZE
	ret

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
