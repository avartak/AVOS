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
	
	mov  bx, ss
	mov  ds, bx
	mov  es, bx
	mov  fs, bx
	mov  gs, bx
	
	inc  DWORD [Interrupt_kernel_reentries]
	jnz  .calltohandler
	
	mov  esp, [Interrupt_stack]
	
	.calltohandler:
	push dword %1
	call Interrupt_Handle
	add  esp, 4
	
	cmp  DWORD [Interrupt_kernel_reentries], 0
	jnz  .restoreandreturn	
	
	cmp  DWORD [Process_next], 0xFFFFFFFF
	je   .loadcurprocstack
	
	mov  ebx, [Process_next]
	mov  [Process_current], ebx
	mov  [Process_next], DWORD 0xFFFFFFFF
	
	.loadcurprocstack:
	push Process_current
	call Paging_SetupDirectory
	call Process_GetKernelSwitchStackLocation
	mov  esp, [Process_current]	
	mov  [TSS_seg+4], eax
	
	.restoreandreturn:
	dec  DWORD [Interrupt_kernel_reentries]
	pop  ebx
	mov  cr3, ebx
	pop  ss
	pop  gs
	pop  fs
	pop  es
	pop  ds
	popad
	
	add  esp, 4
	
	iret

%endmacro

section .text

Interrupt_HandlerForNoErrorCode 0x0
Interrupt_HandlerForNoErrorCode 0x1
Interrupt_HandlerForNoErrorCode 0x2
Interrupt_HandlerForNoErrorCode 0x3
Interrupt_HandlerForNoErrorCode 0x4
Interrupt_HandlerForNoErrorCode 0x5
Interrupt_HandlerForNoErrorCode 0x6
Interrupt_HandlerForNoErrorCode 0x7
Interrupt_HandlerWithErrorCode  0x8
Interrupt_HandlerForNoErrorCode 0x9
Interrupt_HandlerWithErrorCode  0xA
Interrupt_HandlerWithErrorCode  0xB
Interrupt_HandlerWithErrorCode  0xC
Interrupt_HandlerWithErrorCode  0xD
Interrupt_HandlerWithErrorCode  0xE
Interrupt_HandlerForNoErrorCode 0xF
Interrupt_HandlerForNoErrorCode 0x10
Interrupt_HandlerWithErrorCode  0x11
Interrupt_HandlerForNoErrorCode 0x12
Interrupt_HandlerForNoErrorCode 0x13
Interrupt_HandlerForNoErrorCode 0x14
Interrupt_HandlerForNoErrorCode 0x15
Interrupt_HandlerForNoErrorCode 0x16
Interrupt_HandlerForNoErrorCode 0x17
Interrupt_HandlerForNoErrorCode 0x18
Interrupt_HandlerForNoErrorCode 0x19
Interrupt_HandlerForNoErrorCode 0x1A
Interrupt_HandlerForNoErrorCode 0x1B
Interrupt_HandlerForNoErrorCode 0x1C
Interrupt_HandlerForNoErrorCode 0x1D
Interrupt_HandlerWithErrorCode  0x1E
Interrupt_HandlerForNoErrorCode 0x1F

Interrupt_HandlerForNoErrorCode 0x20
Interrupt_HandlerForNoErrorCode 0x21
Interrupt_HandlerForNoErrorCode 0x22
Interrupt_HandlerForNoErrorCode 0x23
Interrupt_HandlerForNoErrorCode 0x24
Interrupt_HandlerForNoErrorCode 0x25
Interrupt_HandlerForNoErrorCode 0x26
Interrupt_HandlerForNoErrorCode 0x27
Interrupt_HandlerForNoErrorCode 0x28
Interrupt_HandlerForNoErrorCode 0x29
Interrupt_HandlerForNoErrorCode 0x2A
Interrupt_HandlerForNoErrorCode 0x2B
Interrupt_HandlerForNoErrorCode 0x2C
Interrupt_HandlerForNoErrorCode 0x2D
Interrupt_HandlerForNoErrorCode 0x2E
Interrupt_HandlerForNoErrorCode 0x2F

Interrupt_HandlerForNoErrorCode 0x80


global Interrupt_IsFlagSet
Interrupt_IsFlagSet:
	pushfd
	pop  eax
	test eax, 0x200
	jz   .ret0
	mov  eax, 1
	ret
	.ret0:
	mov  eax, 0
	ret

