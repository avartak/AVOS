#include <kernel/include/drivers.h>

#include <x86/drivers/include/keyboard.h>

void Drivers_Load() {
	
	Keyboard_Initialize();

}
