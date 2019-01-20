#include <x86/drivers/include/keyboard.h>
#include <x86/kernel/include/io.h>

uint8_t ReadKeyboardScanCode() {

	return Inb(0x60);

}

