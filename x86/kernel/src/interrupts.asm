%macro Interrupt_HandlerForNoErrorCode 1
    global Interrupt_%1 
    Interrupt_%1:
        push dword 0
		Interrupt_DoCommonHandling %1
%endmacro

%macro Interrupt_HandlerWithErrorCode 1
	global Interrupt_%1
    Interrupt_%1:
		Interrupt_DoCommonHandling %1
%endmacro

%macro Interrupt_DoCommonHandling 1

	extern Paging_SetupDirectory
	extern Interrupt_Handle
	extern Interrupt_kernel_reentries
	extern Interrupt_stack
	extern Process_GetKernelSwitchStackLocation
	extern Process_current
	extern Process_next
	extern TSS_seg

	pushad
	push ds
	push es
	push fs
	push gs
	push ss
	mov  ebx, cr3
	push ebx

	mov bx, ss
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx

	inc DWORD [Interrupt_kernel_reentries]
	jnz .calltohandler

	mov esp, [Interrupt_stack]

	.calltohandler:
	push dword 0x%1
	call Interrupt_Handle
	add  esp, 4

	cmp DWORD [Interrupt_kernel_reentries], 0
	jnz .restoreandreturn	
	
	cmp DWORD [Process_next], 0xFFFFFFFF
	je .loadcurprocstack

	mov ebx, [Process_next]
	mov [Process_current], ebx
	mov [Process_next], DWORD 0xFFFFFFFF

	.loadcurprocstack:
	push Process_current
	call Paging_SetupDirectory
	call Process_GetKernelSwitchStackLocation
	mov  esp, [Process_current]	
	mov  [TSS_seg+4], eax

	.restoreandreturn:
	dec DWORD [Interrupt_kernel_reentries]
	pop ebx
	mov cr3, ebx
	pop ss
	pop gs
	pop fs
	pop es
	pop ds
	popad
	
	add esp, 4

	iret

%endmacro

section .text

Interrupt_HandlerForNoErrorCode  0
Interrupt_HandlerForNoErrorCode  1
Interrupt_HandlerForNoErrorCode  2
Interrupt_HandlerForNoErrorCode  3
Interrupt_HandlerForNoErrorCode  4
Interrupt_HandlerForNoErrorCode  5
Interrupt_HandlerForNoErrorCode  6
Interrupt_HandlerForNoErrorCode  7
Interrupt_HandlerWithErrorCode   8
Interrupt_HandlerForNoErrorCode  9
Interrupt_HandlerWithErrorCode   A
Interrupt_HandlerWithErrorCode   B
Interrupt_HandlerWithErrorCode   C
Interrupt_HandlerWithErrorCode   D
Interrupt_HandlerWithErrorCode   E
Interrupt_HandlerForNoErrorCode  F
Interrupt_HandlerForNoErrorCode 10
Interrupt_HandlerWithErrorCode  11
Interrupt_HandlerForNoErrorCode 12
Interrupt_HandlerForNoErrorCode 13
Interrupt_HandlerForNoErrorCode 14
Interrupt_HandlerForNoErrorCode 15
Interrupt_HandlerForNoErrorCode 16
Interrupt_HandlerForNoErrorCode 17
Interrupt_HandlerForNoErrorCode 18
Interrupt_HandlerForNoErrorCode 19
Interrupt_HandlerForNoErrorCode 1A
Interrupt_HandlerForNoErrorCode 1B
Interrupt_HandlerForNoErrorCode 1C
Interrupt_HandlerForNoErrorCode 1D
Interrupt_HandlerWithErrorCode  1E
Interrupt_HandlerForNoErrorCode 1F

Interrupt_HandlerForNoErrorCode 20
Interrupt_HandlerForNoErrorCode 21
Interrupt_HandlerForNoErrorCode 22
Interrupt_HandlerForNoErrorCode 23
Interrupt_HandlerForNoErrorCode 24
Interrupt_HandlerForNoErrorCode 25
Interrupt_HandlerForNoErrorCode 26
Interrupt_HandlerForNoErrorCode 27
Interrupt_HandlerForNoErrorCode 28
Interrupt_HandlerForNoErrorCode 29
Interrupt_HandlerForNoErrorCode 2A
Interrupt_HandlerForNoErrorCode 2B
Interrupt_HandlerForNoErrorCode 2C
Interrupt_HandlerForNoErrorCode 2D
Interrupt_HandlerForNoErrorCode 2E
Interrupt_HandlerForNoErrorCode 2F

Interrupt_HandlerForNoErrorCode 80


global Interrupt_IsFlagSet
Interrupt_IsFlagSet:
	pushfd
	pop eax
	test eax, 0x200
	jz .ret0
	mov eax, 1
	ret
	.ret0:
	mov eax, 0
	ret

