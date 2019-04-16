#include <x86/boot/include/bios.h>

void BIOS_ClearRegistry(struct BIOS_Registers* regs) {

    regs->eax   = 0;
    regs->ebx   = 0;
    regs->ecx   = 0;
    regs->edx   = 0;
    regs->esi   = 0;
    regs->edi   = 0;
    regs->ebp   = 0;
    regs->esp   = 0;
    regs->ds    = 0;
    regs->es    = 0;
    regs->ss    = 0;
    regs->flags = 0;

    return;
}

