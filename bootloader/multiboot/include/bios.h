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

#ifndef BOOTLOADER_BIOS_H
#define BOOTLOADER_BIOS_H

#include <bootloader/multiboot/include/defs.h>

struct BIOS_Registers {
	union {
    	uint32_t eax;
		uint16_t ax;
		struct {
			uint8_t al;
			uint8_t ah;
		}__attribute__((packed));
	};

    union {
        uint32_t ebx;
        uint16_t bx;
        struct {
            uint8_t bl;
            uint8_t bh;
        }__attribute__((packed));
    };

    union {
        uint32_t ecx;
        uint16_t cx;
        struct {
            uint8_t cl;
            uint8_t ch;
        }__attribute__((packed));
    };

    union {
        uint32_t edx;
        uint16_t dx;
        struct {
            uint8_t dl;
            uint8_t dh;
        }__attribute__((packed));
    };

    union {
        uint32_t esi;
        uint16_t si;
    };

    union {
        uint32_t edi;
        uint16_t di;
    };

    union {
        uint32_t ebp;
        uint16_t bp;
    };

    union {
        uint32_t esp;
        uint16_t sp;
    };

    uint16_t  ds;
    uint16_t  es;
    uint16_t  ss;

    uint16_t flags;

}__attribute__((packed));

extern void BIOS_ClearRegistry(struct BIOS_Registers* regs);
extern void BIOS_Interrupt(uint32_t interrupt, struct BIOS_Registers* regs);

#endif

