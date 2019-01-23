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

    .poststack:
    push dword 0x%1

    extern Interrupt_Handler
    call   Interrupt_Handler

    add  esp, 4
    add  esp, 4
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

