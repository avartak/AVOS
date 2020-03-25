BITS 32

section .init

global Initialize_CRT
Initialize_CRT:
	push ebp
	mov  ebp, esp

section .fini
global Finish_CRT
Finish_CRT:
	push ebp
	mov  ebp, esp
