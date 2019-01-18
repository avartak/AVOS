%macro HandlerForInterruptWithNoErrorCode 1
    global Interrupt%1
    Interrupt%1:
        push dword 0
		DoCommonInterruptHandling %1
%endmacro

%macro HandlerForInterruptWithErrorCode 1
	global Interrupt%1
    Interrupt%1:
		DoCommonInterruptHandling %1
%endmacro

%macro DoCommonInterruptHandling 1
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp                                      ; This is probably redundant -- the C function will do it anyways ?

	push dword 0x%1

	extern HandleInterrupt
    call   HandleInterrupt

    add     esp, 4

    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax

    add     esp, 4
	iret
%endmacro

section .text

HandlerForInterruptWithNoErrorCode  0
HandlerForInterruptWithNoErrorCode  1
HandlerForInterruptWithNoErrorCode  2
HandlerForInterruptWithNoErrorCode  3
HandlerForInterruptWithNoErrorCode  4
HandlerForInterruptWithNoErrorCode  5
HandlerForInterruptWithNoErrorCode  6
HandlerForInterruptWithErrorCode    7
HandlerForInterruptWithNoErrorCode  8
HandlerForInterruptWithErrorCode    9
HandlerForInterruptWithErrorCode    A
HandlerForInterruptWithErrorCode    B
HandlerForInterruptWithErrorCode    C
HandlerForInterruptWithErrorCode    D
HandlerForInterruptWithNoErrorCode  E
HandlerForInterruptWithNoErrorCode  F
HandlerForInterruptWithErrorCode   10
HandlerForInterruptWithNoErrorCode 11
HandlerForInterruptWithNoErrorCode 12
HandlerForInterruptWithNoErrorCode 13
HandlerForInterruptWithNoErrorCode 14
HandlerForInterruptWithNoErrorCode 15
HandlerForInterruptWithNoErrorCode 16
HandlerForInterruptWithNoErrorCode 17
HandlerForInterruptWithNoErrorCode 18
HandlerForInterruptWithNoErrorCode 19
HandlerForInterruptWithNoErrorCode 1A
HandlerForInterruptWithNoErrorCode 1B
HandlerForInterruptWithNoErrorCode 1C
HandlerForInterruptWithNoErrorCode 1D
HandlerForInterruptWithNoErrorCode 1E
HandlerForInterruptWithNoErrorCode 1F

HandlerForInterruptWithNoErrorCode 20
HandlerForInterruptWithNoErrorCode 21
HandlerForInterruptWithNoErrorCode 22
HandlerForInterruptWithNoErrorCode 23
HandlerForInterruptWithNoErrorCode 24
HandlerForInterruptWithNoErrorCode 25
HandlerForInterruptWithNoErrorCode 26
HandlerForInterruptWithNoErrorCode 27
HandlerForInterruptWithNoErrorCode 28
HandlerForInterruptWithNoErrorCode 29
HandlerForInterruptWithNoErrorCode 2A
HandlerForInterruptWithNoErrorCode 2B
HandlerForInterruptWithNoErrorCode 2C
HandlerForInterruptWithNoErrorCode 2D
HandlerForInterruptWithNoErrorCode 2E
HandlerForInterruptWithNoErrorCode 2F

HandlerForInterruptWithNoErrorCode 80


