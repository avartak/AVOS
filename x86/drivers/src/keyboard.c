#include <x86/drivers/include/keyboard.h>
#include <x86/drivers/include/io.h>

void Keyboard_HandleInterrupt() {
	Inb(0x60);
	return;
}

