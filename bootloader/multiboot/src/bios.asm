; This is the implementation of two functions that allow for the capability of calling real-mode BIOS routines from 32-bit C code
; See bios.h for more details about the functions 

; The following are the descriptors for the 32-bit and 16-bit protected mode segments that the bootloader should have alreay set up

SEG32_CODE           equ 0x08
SEG32_DATA           equ 0x10
SEG16_CODE           equ 0x18
SEG16_DATA           equ 0x20

; We will have a separate stack for the real-mode code
; The stack segment will be set to memory location BIOS_Stack_Segment (defined at the end of this file)
; The stack pointer will be set to memory location BIOS_Stack_Pointer (defined at the end of this file)
; The constant REAL_MODE_STACK_SIZE gives the size of the stack - it is set to 0x400 bytes or 1 KB

REAL_MODE_STACK_SIZE equ 0x400

; In the FLAGS register the bit mask 0x200 can be used to test the status of the interrupt flag 
; When switching to real mode, we will clear the interrupt flag
; When switching back to protected mode, we will choose to reenable interrupts depending on the status of the interrupt flag in the saved FLAGS register

INTERRUPTS_ENABLED   equ 0x200

; We expect the bootloader code to reside in low memory, so below the 1 MB address limit
; But labels/pointers may point to addresses greater than 64 KB i.e. larger than 16-bit values
; The following macro computes the memory address of a label relative to the start of the code section defined in this file. 
; By appropriately setting the segment registers, this 'REL_ADDR' should always yield a 16-bit value (i.e. a value smaller than 64 KB)

%define REL_ADDR(x)  (x-BIOS_Start)

section .text

BITS 32

; We align the start of this section on a 16-byte boundary. 
; This way we can simply set a segment register to point to the start of the code section by shifting BIOS_Start to the right by 4 bits and saving this value in the segment register
; In the code that follows we will set two segment register to point to BIOS_Start - FS and CS
; The real mode stack is also aligned on a 16-byte boundary and so SS is also set in a similar way to point to BIOS_Stack_Segment

align 0x10

BIOS_Start:

; The 40-byte "BIOS Registry" structure
; Saves all the general purpose registers, segment registers and FLAGS register

BIOS_Regs16:
	.eax    dd   0
	.ebx    dd   0
	.ecx    dd   0
	.edx    dd   0
	.esi    dd   0
	.edi    dd   0
	.ebp    dd   0
	.esp    dd   0
	.ds     dw   0
	.es     dw   0
	.ss     dw   0
	.flags  dw   0

; Interrupt number to be invoked

BIOS_Int_ID db 0

; Place to store the protected mode stack pointer (we will use a dedicated 1 KB stack for the real mode)

BIOS_PM_ESP dd   0 

global BIOS_ClearRegistry
; Parameters (in order that is passed when calling the function in C):
; - Pointer to the BIOS registry structure that needs to be cleared
BIOS_ClearRegistry:

	; Set EBP as the pointer to the stack frame

	push  ebp
	mov   ebp, esp

	; We need to save the state of EDI as per the C calling convention in the System V ABI

	push  edi

	; We clear the contents of the BIOS registry whose pointer is passed as an argument to this function

	mov   edi, [ebp+8]
	mov   eax, 0
	mov   ecx, 10
	rep   stosd

	; Restoring the state of EDI

	pop   edi

	; Restoring the stack pointer and EBP and returning 

	mov   esp, ebp
	pop   ebp
	ret

global BIOS_Interrupt
; Parameters (in order that is passed when calling the function in C):
; - Interrupt ID
; - Pointer to the BIOS registry structure
BIOS_Interrupt:

	; Set EBP as the pointer to the stack frame

	push  ebp
	mov   ebp, esp

	; We start by saving the general registers and the flags register on the stack (pushing the flags register twice is deliberate - used at the end to set the interrupt flag)

	pushfd
	pushfd
	pushad

	; Clear interrupts

	cli

	; Save the interrupt vector index

	mov   al, [ebp+8]
	mov   [BIOS_Int_ID], al

	; Save the real mode register values to be used when calling the BIOS routine
	; These are stored in a data structure referred to as BIOS_Registers in the C code
	; A pointer to the BIOS_Registers data structure containing the register values (system state) needed when calling the BIOS routine is passed to this function
	; Copy these register values to a similar 'BIOS_Regs16' data structure we have created above - this BIOS registry has a size of 40 bytes

	mov   esi, [ebp+12]
	mov   edi, BIOS_Regs16
	mov   ecx, 10
	rep   movsd

	; Before switching to the real mode to run the BIOS routine, we first save the stack pointer of our 32-bit code
	
	mov   [BIOS_PM_ESP], esp

	; Switch to 16-bit protected mode by doing a far jump with a 16-bit protected mode code segment

	jmp   SEG16_CODE:BIOS_Interrupt.PMode16

BITS 16

	; Switch the segment registers to 16-bit protected mode (1 MB address space)
	
	.PMode16:
	mov   ax, SEG16_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	
	; Switch to real mode by disabling the PE bit in the CR0 register
	
	mov   eax, cr0
	and   al , 0xFE
	mov   cr0, eax
	
	; Set up a real mode stack
	
	mov   eax, BIOS_Stack_Segment
	shr   eax, 4
	mov   ss, ax
	mov   esp, BIOS_Stack_Pointer-BIOS_Stack_Segment
	
	; Set the base address of data segments (DS,ES,GS) to 0
	
	xor   ax, ax
	mov   ds, ax
	mov   es, ax
	mov   gs, ax
	
	; Set the base address of FS to BIOS_Start - we will use FS as the base segment for access all memory addresses/labels in the real-mode code
	
	mov   eax, BIOS_Start
	shr   eax, 4
	mov   fs, ax
	
	; Switch the code segment register to real mode with a far return, setting the base address of CS to BIOS_Start (same as FS)
	; The following three lines of code provide a way to perform a far jump by changing CS to some desired value
	
	push  fs
	push  REL_ADDR(BIOS_Interrupt.RMode)
	retf
	
	.RMode:
	
	; Save the real mode stack pointer in the BIOS registry
	
	mov   [fs:REL_ADDR(BIOS_Regs16.esp)], esp
	
	; Get the pointer to the interrupt routine in SI (4 bytes : first two bytes offset, next two bytes segment)
	; The real mode interrupt vector table (IVT) is a 0x400 byte table at memory addresses 0-0x400
	; It is a vector of 0x100 (256) entries for the 0x100 possible interrupts	
	; Each 4-byte entry consists of the offset and segment values that together point to the interrupt handler code

	mov   al, BYTE [fs:REL_ADDR(BIOS_Int_ID)]
	mov   bl, 4
	mul   bl
	mov   si, ax
	
	; Set up the stack to return from the interrupt routine at the appropriate point
	; Since the BIOS routine will use an 'iret' (interrupt return) to return back to the calling code, we need to 
	; - push the FLAGS register on the stack 
	; - then the code segment
	; - And lastly, the offset (w.r.t. the code segment) of the code that the BIOS routine should return to 

	pushf
	push  cs
	push  WORD REL_ADDR(BIOS_Interrupt.SwitchToPMode32)

	; Push the address (segment and offset) of the interrupt routine on the stack so that we can call the routine with a far return

	push  DWORD [si]
	
	; Before we call the BIOS routine we need to change the state of the registers to appropriate values that the BIOS routine is expected to get as inputs
	; Store the designated values in DS and ES (taken from BIOS_Regs16)
	
	mov   ax, [fs:REL_ADDR(BIOS_Regs16.ds)]
	mov   ds, ax
	mov   ax, [fs:REL_ADDR(BIOS_Regs16.es)]
	mov   es, ax
	
	; Store the designated values in the general purpose registers
	
	mov   eax, [fs:REL_ADDR(BIOS_Regs16.eax)]
	mov   ebx, [fs:REL_ADDR(BIOS_Regs16.ebx)]
	mov   ecx, [fs:REL_ADDR(BIOS_Regs16.ecx)]
	mov   edx, [fs:REL_ADDR(BIOS_Regs16.edx)]
	mov   esi, [fs:REL_ADDR(BIOS_Regs16.esi)]
	mov   edi, [fs:REL_ADDR(BIOS_Regs16.edi)]
	mov   ebp, [fs:REL_ADDR(BIOS_Regs16.ebp)]
	
	; We are ready now to trigger the appropriate BIOS interrupt
	
	clc 
	retf
	
	; We are done with the BIOS routine; now we have to switch back to the protected mode
	; First we restore the stack pointer and store the register values (these represent the state immediately after the BIOS call) in the BIOS_Regs16 structure
	
	.SwitchToPMode32:
	mov   esp, [fs:REL_ADDR(BIOS_Regs16.esp)]
	mov   [fs:REL_ADDR(BIOS_Regs16.eax)], eax	
	mov   [fs:REL_ADDR(BIOS_Regs16.ebx)], ebx	
	mov   [fs:REL_ADDR(BIOS_Regs16.ecx)], ecx	
	mov   [fs:REL_ADDR(BIOS_Regs16.edx)], edx	
	mov   [fs:REL_ADDR(BIOS_Regs16.esi)], esi	
	mov   [fs:REL_ADDR(BIOS_Regs16.edi)], edi	
	mov   [fs:REL_ADDR(BIOS_Regs16.ebp)], ebp	
	
	; Also save the flags register
	
	pushf
	pop   ax
	mov   [fs:REL_ADDR(BIOS_Regs16.flags)], ax
	
	; Save the segment registers
	
	mov   ax, ds
	mov   [fs:REL_ADDR(BIOS_Regs16.ds)], ax	
	mov   ax, es
	mov   [fs:REL_ADDR(BIOS_Regs16.es)], ax	
	
	; Switch to 32-bit protected mode
	
	mov   eax, cr0
	or    al , 1
	mov   cr0, eax
	
	; Switch the code segment to 32-bit protected mode by making a long jump

	jmp   SEG32_CODE:BIOS_Interrupt.PMode32

BITS 32

	.PMode32:
	
	; Switch data segments to the 32-bit protected mode and restore the general purpose registers to their values at the time this function was called
	
	mov   ax, SEG32_DATA
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	
	; Restore the stack pointer to its initial position in the 32-bit code

	mov   esp, [BIOS_PM_ESP]

	; Restore all the registers (including FLAGS) back to the values they were carrying in the 32-bit code
	; Note that POPFD will not restore all the flags (e.g. the interrupt flag we disabled while entering the real mode will not be restored in case interrupts were initially enabled)

	popad
	popfd
	
	; Save the real mode register values in the structure whose pointer that was passed to this function as an argument
	; We basically copy the contents of BIOS_Regs to the registry structure whose pointer was passed as an argument	

	push  esi
	push  edi
	push  ecx
	
	mov   esi, BIOS_Regs16
	mov   edi, [ebp+12]
	mov   ecx, 10
	rep   movsd
	
	pop   ecx
	pop   edi
	pop   esi
	
	; Check if interrupts were enabled when BIOS_Interrupt was called, and if so, enable them again now
	
	pop   eax
	test  eax, INTERRUPTS_ENABLED
	jz    .return
	sti
	
	; Return 
	
	.return:
	mov   esp, ebp
	pop   ebp
	ret



align 0x10

BIOS_Stack_Segment:
times REAL_MODE_STACK_SIZE db   0
BIOS_Stack_Pointer:

