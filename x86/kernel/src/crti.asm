BITS 32

section .init

global CRT_Initialize
CRT_Initialize:
	push ebp
	mov  ebp, esp

section .fini
global CRT_Finish
CRT_Finish:
	push ebp
	mov  ebp, esp
