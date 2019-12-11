/*

These functions can be used to call real-mode BIOS routines from 32-bit C code. 

A data structure called 'BIOS_Registers' is defined which stores the state of the general purpose registers that needs to be passed to the BIOS routines.
It also saves the state of these registers as a result of the BIOS routine call

The function 'BIOS_ClearRegistry' simply clears the contents of all the registers in the BIOS_Registers data structure whose pointer is passed as an argument to the function

The function 'BIOS_Interrupt' triggers the BIOS interrupt. It takes as input 
- the interrupt number to be called 
- the pointer of the BIOS_Registers data structure to be used to set up the registers when calling the BIOS routine

These functions are implemented in assembly code. Take a look at bios.asm to see how they are implemented. 

*/

#ifndef BOOT_BIOS_H
#define BOOT_BIOS_H

#include <boot/include/defs.h>

struct BIOS_Registers {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint16_t  ds;
    uint16_t  es;
    uint16_t  ss;
    uint16_t flags;
}__attribute__((packed));

extern void BIOS_Interrupt(uint32_t interrupt, struct BIOS_Registers* regs);
extern void BIOS_ClearRegistry(struct BIOS_Registers* regs);

#endif

