; The following are the descriptors for the 32-bit and 16-bit protected mode segments

SEG32_CODE        equ 0x08
SEG32_DATA        equ 0x10
SEG16_CODE        equ 0x18
SEG16_DATA        equ 0x20

BITS 32

global BIOS_Interrupt
BIOS_Interrupt:

	; We start by saving the general registers to memory, in the BIOS_Regs32 structure defined at the end

	mov   [BIOS_Regs32.eax], eax
	mov   [BIOS_Regs32.ebx], ebx
	mov   [BIOS_Regs32.ecx], ecx
	mov   [BIOS_Regs32.edx], edx
	mov   [BIOS_Regs32.esi], esi
	mov   [BIOS_Regs32.edi], edi
	mov   [BIOS_Regs32.ebp], ebp
	mov   [BIOS_Regs32.esp], esp

	; We then save the IDT descriptor

	sidt  [Save_IDT_Desc]

	; We then copy the interrupt vector index to the EDX register

	mov   edx, [esp+4]
	mov   [BIOS_Int_ID], edx

	; Next, we copy to EAX the pointer to the structure containing the general purpose register values 
	; to be used in the real mode when calling some BIOS interrupt

	mov   eax, [esp+8]

	; We store the real mode register values (from the structure pointed to by EAX) into the BIOS_Regs16 structure

	mov   edx, [eax+0x00]
	mov   [BIOS_Regs16.eax], edx
	mov   edx, [eax+0x04]
	mov   [BIOS_Regs16.ebx], edx
	mov   edx, [eax+0x08]
	mov   [BIOS_Regs16.ecx], edx
	mov   edx, [eax+0x0C]
	mov   [BIOS_Regs16.edx], edx
	mov   edx, [eax+0x10]
	mov   [BIOS_Regs16.esi], edx
	mov   edx, [eax+0x14]
	mov   [BIOS_Regs16.edi], edx
	mov   edx, [eax+0x18]
	mov   [BIOS_Regs16.ebp], edx

	mov   dx , [eax+0x20]
	mov   [BIOS_Regs16.ds], dx
	mov   dx , [eax+0x22]
	mov   [BIOS_Regs16.es], dx

	; Switch to 16-bit protected mode

	jmp   SEG16_CODE:BIOS_Interrupt.PMode16

BITS 16

	; Switch the segment registers to 16-bit protected mode (1 MB address space)

	.PMode16:
	mov   dx, SEG16_DATA
	mov   ds, dx
	mov   es, dx
	mov   fs, dx
	mov   gs, dx
	mov   ss, dx

	; Switch to real mode by disabling the PE bit in the CR0 register

	mov   edx, cr0
	and   dl , 0xFE
	mov   cr0, edx

	; Switch the code segment register to real mode

	jmp   0:BIOS_Interrupt.RMode

	; Switch the segment registers to real mode

	.RMode:
	mov   dx, 0
	mov   ds, dx
	mov   es, dx
	mov   fs, dx
	mov   gs, dx
	mov   ss, dx

	; Store the designated values in DS and ES (taken from BIOS_Regs16)

	mov   dx, [fs:BIOS_Regs16.ds]
	mov   ds, dx
	mov   dx, [fs:BIOS_Regs16.es]
	mov   es, dx

	; Store the designated values in the general purpose registers
	; We do not change the stack pointer ; it could be done but we do not need to do it in the context of our boot loader set up

	mov   eax, [fs:BIOS_Regs16.eax]
	mov   ebx, [fs:BIOS_Regs16.ebx]
	mov   ecx, [fs:BIOS_Regs16.ecx]
	mov   edx, [fs:BIOS_Regs16.edx]
	mov   esi, [fs:BIOS_Regs16.esi]
	mov   edi, [fs:BIOS_Regs16.edi]
	mov   ebp, [fs:BIOS_Regs16.ebp]

	; Save the stack pointer in case the BIOS routine trashes it

	mov   [fs:BIOS_Regs16.esp], esp

	; Load the real mode IDT so that we can call the BIOS interrupts
	; In general, we should save the 32-bit IDT descriptor so that we can use it after switching back to protected mode 
	; But we don't have it set up in the boot loader anyways

	lidt  [fs:BIOS_IDT_Desc]

	; Trigger the appropriate BIOS interrupt
	; It is at this stage that we will change the DS register to the designated value

	cmp   WORD [fs:BIOS_Int_ID], 0x10
	je    BIOS_Interrupt.Int0x10
	cmp   WORD [fs:BIOS_Int_ID], 0x13
	je    BIOS_Interrupt.Int0x13
	cmp   WORD [fs:BIOS_Int_ID], 0x15
	je    BIOS_Interrupt.Int0x15

	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x10:
	int   0x10
	jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x13:
    int   0x13
    jmp   BIOS_Interrupt.SwitchToPMode32

	.Int0x15:
    int   0x15
    jmp   BIOS_Interrupt.SwitchToPMode32


	; We are done with the BIOS routine; now we have to switch back to the protected mode
	; First we restore the stack pointer and store the register values (these represent the state immediately after the BIOS call) in the BIOS_Regs16 structure

	.SwitchToPMode32:
    mov   esp, [fs:BIOS_Regs16.esp]
	mov   [fs:BIOS_Regs16.eax], eax	
	mov   [fs:BIOS_Regs16.ebx], ebx	
	mov   [fs:BIOS_Regs16.ecx], ecx	
	mov   [fs:BIOS_Regs16.edx], edx	
	mov   [fs:BIOS_Regs16.esi], esi	
	mov   [fs:BIOS_Regs16.edi], edi	
	mov   [fs:BIOS_Regs16.ebp], ebp	

	; Also save the flags register

	pushfd
	pop   edx
	mov   [fs:BIOS_Regs16.flags], dx

	; Save the segment registers

	mov   dx, ds
	mov   [fs:BIOS_Regs16.ds], dx	
	mov   dx, es
	mov   [fs:BIOS_Regs16.es], dx	

	; Switch to 32-bit protected mode

    mov   edx, cr0
    or    dl , 1
    mov   cr0, edx

    jmp   SEG32_CODE:BIOS_Interrupt.PMode32

BITS 32

	.PMode32:

	; Switch data segments to the 32-bit protected mode

	mov   dx, SEG32_DATA
	mov   ds, dx
	mov   es, dx
	mov   fs, dx
	mov   gs, dx
	mov   ss, dx
	mov   esp, [BIOS_Regs32.esp]	

	; Save the real mode register values in the structure whose pointer that was passed to this function as an argument

    mov   eax, [esp+8]
    mov   edx, [BIOS_Regs16.eax]
    mov   [eax+0x00], edx
    mov   edx, [BIOS_Regs16.ebx]
    mov   [eax+0x04], edx
    mov   edx, [BIOS_Regs16.ecx]
    mov   [eax+0x08], edx
    mov   edx, [BIOS_Regs16.edx]
    mov   [eax+0x0C], edx
    mov   edx, [BIOS_Regs16.esi]
    mov   [eax+0x10], edx
    mov   edx, [BIOS_Regs16.edi]
    mov   [eax+0x14], edx
    mov   edx, [BIOS_Regs16.ebp]
    mov   [eax+0x18], edx

    mov   dx , [BIOS_Regs16.ds]
    mov   [eax+0x20], dx
    mov   dx , [BIOS_Regs16.es]
    mov   [eax+0x22], dx
    mov   dx , [BIOS_Regs16.flags]
    mov   [eax+0x26], dx

	; Restore the general purpose registers to their values at the time this function was called

	mov   eax, [BIOS_Regs32.eax]	
	mov   ebx, [BIOS_Regs32.ebx]	
	mov   ecx, [BIOS_Regs32.ecx]	
	mov   edx, [BIOS_Regs32.edx]	
	mov   esi, [BIOS_Regs32.esi]	
	mov   edi, [BIOS_Regs32.edi]	
	mov   ebp, [BIOS_Regs32.ebp]	
	mov   esp, [BIOS_Regs32.esp]	

	; Restore the IDT that we started out with 

	lidt  [Save_IDT_Desc]

	; Return 

	ret



section .data

BIOS_Regs32:
	.eax    dd   0
	.ebx    dd   0
	.ecx    dd   0
	.edx    dd   0
	.esi    dd   0
	.edi    dd   0
	.ebp    dd   0
	.esp    dd   0


BIOS_Regs16:
	.eax    dd   0
	.ebx    dd   0
	.ecx    dd   0
	.edx    dd   0
	.esi    dd   0
	.edi    dd   0
	.ebp    dd   0
	.esp    dd   0
	.flags  dw   0
	.ds     dw   0
	.es     dw   0
	.ss     dw   0

BIOS_Int_ID dd 0


BIOS_IDT_Desc:
    dw 0x400 - 1
    dd 0


Save_IDT_Desc:
    dw 0x400 - 1
    dd 0


