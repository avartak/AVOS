#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/welcome.h>
#include <x86/drivers/include/pic.h>
#include <x86/drivers/include/pit.h>
#include <x86/drivers/include/keyboard.h>
#include <kernel/include/interrupts.h>

void Kinit() {

	GDT_Initialize();
	
	IDT_Initialize();
	
	Physical_Memory_Initialize();
	
	Interrupt_Initialize();
	
	PIC_Initialize();
	
	PIT_Initialize(PIT_TARGET_FREQUENCY);
	
	Keyboard_Initialize(PIT_TARGET_FREQUENCY);
	
	Welcome();

}
