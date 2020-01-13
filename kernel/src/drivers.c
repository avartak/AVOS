#include <kernel/include/drivers.h>

#include <kernel/x86/drivers/include/pic.h>
#include <kernel/x86/drivers/include/pit.h>
#include <kernel/x86/drivers/include/keyboard.h>

void Drivers_Load() {
	
	PIC_Initialize();
	
	PIT_Initialize();
	
	Keyboard_Initialize();

}
