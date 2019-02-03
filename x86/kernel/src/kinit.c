/*

Kinit done all the low level initialization of the system to set it up for OS business.
Here is what is done :
- Set up the GDT. The boot loader guarantees that we are in protected mode with some well defined segments in place. However, that's just temporary. We need to set up the GDT for ourselves
- Set up paging. We will map the kernel to higher half (3 GB)
- Set up the IDT. We will set up the framework for interrupt handling.
- Set up the PIC so that we can start accepting interrupts from external devices (however, for starters we will keep them disabled)
- Enable interrupts. However, not much will be in place for handling them. 
- Show a nice welcome message to be sure that everything is going according to plan
- Start enabling the external interrupts one by one. For now, we just enable the keyboad

*/


#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/tss.h>
#include <x86/kernel/include/paging.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/interrupts.h>
#include <x86/kernel/include/pic.h>
#include <x86/kernel/include/misc.h>
#include <x86/kernel/include/welcome.h>

void Kinit() {

	// Setup the GDT (again)
	GDT_Initialize();

	// Setup the TSS 
	TSS_Initialize();

	// This is an empty function right now
	Paging_Initialize();

	// Prepare the physical memory map and setup the low level page allocation function of the kernel
	Physical_Memory_Initialize();

	// Setup the IDT : First 32 interrupts for the CPU, the next 16 interrupts for the PIC, and interrupt 0x80 for syscalls -- none of these are implemented, just allocated
	IDT_Initialize();

	// Initialize the PIC -- remap the master and slave IRQs to interrupt vectors 0x20-0x27 and 0x28-0x30 respectively ; Mask all interrupts
	PIC_DisableAllInterrupts();
	PIC_Initialize(PIC_REMAP1_START, PIC_REMAP2_START);

	// Enable interrupts -- note the interrupts in the PIC are all still masked ; The CPU is now willing to listen but PIC will not be sending anything right now
	EnableInterrupts();

	// Display welcome message
	Welcome();

	// We now enable the keyboard interrupt
	PIC_EnableInterrupt(1);

    return;
}

