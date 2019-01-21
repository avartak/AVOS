#include <x86/kernel/include/interrupts.h>
#include <x86/kernel/include/pic.h>
#include <x86/drivers/include/keyboard.h>

#include <stddef.h>

int screenpos = 2;

void HandleInterrupt(uint32_t interrupt, struct StackStateAtInterrupt stack, struct CPUStateAtInterrupt cpu) {

    int syscallid = 0;

    if (stack.error_code != 0) {
    }

    if (interrupt >= 0x00 && interrupt < 0x20) {
		IRQTest("CPU has raised an exception", 0x04);
    }

    if (interrupt == 0x21) {
		IRQTest("Keyboard interrupt received", 0x0A);
		ReadKeyboardScanCode();
	}

    if (interrupt >= 0x20 && interrupt < 0x30) {
        PIC_SendEOI(interrupt - 0x20);
		IRQTest("EOI command sent to the PIC", 0x0A);
    }

    if (interrupt == 0x80) {
		IRQTest("System call received", 0x0A);
        syscallid = cpu.eax;
    }

	if (interrupt >= 0x30 && interrupt != 0x80) {
		IRQTest("Unrecognized interrupt received", 0x04);
	}

	IRQTest("", 0x00);

    return;

}

void IRQTest(const char* str, uint8_t color) {

	char* screen = (char*)0xC00B8000;
	
	size_t i = 0;
	while (str[i] != 0) {
	    screen[screenpos*160 + 2*i]   = str[i];
	    screen[screenpos*160 + 2*i+1] = color;
	    i++;
	}
	
	screenpos++;

}
