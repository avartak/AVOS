#include <x86/kernel/include/interrupts.h>
#include <x86/kernel/include/pic.h>
#include <x86/drivers/include/keyboard.h>

#include <stddef.h>
#include <stdint.h>

extern uint32_t screen_line;

void Interrupt_Handler(uint32_t interrupt) {

    if (interrupt >= 0x00 && interrupt < 0x20) {
		IRQTest("CPU has raised an exception", 0x04);
    }

    if (interrupt == 0x21) {
		ReadKeyboardScanCode();
	}

    if (interrupt >= 0x20 && interrupt < 0x30) {
        PIC_SendEOI(interrupt - 0x20);
    }

    return;

}

void IRQTest(const char* str, uint8_t color) {

	char* screen = (char*)0xC00B8000;
	
	size_t i = 0;
	while (str[i] != 0) {
	    screen[screen_line*160 + 2*i]   = str[i];
	    screen[screen_line*160 + 2*i+1] = color;
	    i++;
	}
	
	screen_line++;

}
