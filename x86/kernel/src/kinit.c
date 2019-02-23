#include <x86/kernel/include/gdt.h>
#include <x86/kernel/include/tss.h>
#include <x86/kernel/include/idt.h>
#include <x86/kernel/include/physmem.h>
#include <x86/kernel/include/interrupts.h>
#include <x86/kernel/include/welcome.h>
#include <x86/drivers/include/pic.h>
#include <x86/drivers/include/pit.h>
#include <x86/drivers/include/keyboard.h>

void Kinit(uint32_t* boot_info) {

	GDT_Initialize();
	
	TSS_Initialize();
	
	IDT_Initialize();
	
	Physical_Memory_Initialize(boot_info);
	
	Interrupt_Initialize();
	
	PIC_Initialize();
	
	PIT_Initialize(PIT_TARGET_FREQUENCY);
	
	Keyboard_Initialize(PIT_TARGET_FREQUENCY);
	
	Welcome();

}
