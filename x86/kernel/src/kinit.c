#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/paging.h>
#include <x86/kernel/include/welcome.h>

void Kinit() {

	// Setup the GDT (again)
	SetupGDT();

	// Identity map disabled, 4 MB - 8 MB physical memory also mapped to higher half for page tables
	InitPaging();

	// Display welcome message
	Welcome();

    return;
}

