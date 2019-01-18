#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/paging.h>
#include <x86/kernel/include/pic.h>
#include <x86/kernel/include/welcome.h>

void Kinit() {

	// Setup the GDT (again)
	SetupGDT();

	// Identity map disabled, 4 MB - 8 MB physical memory also mapped to higher half for page tables
	InitPaging();

	// Identity map disabled, 4 MB - 8 MB physical memory also mapped to higher half for page tables
	SetupIDT();

	// Initialize the PIC -- remap the master and slave IRQs to interrupt vectors 0x20-0x27 and 0x28-0x30 respectively
	PIC_Init(PIC1_REMAP_START, PIC2_REMAP_START);

	// Display welcome message
	Welcome();

    return;
}

