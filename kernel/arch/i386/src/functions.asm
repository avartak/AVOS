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

