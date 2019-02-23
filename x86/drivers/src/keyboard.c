#include <x86/drivers/include/keyboard.h>
#include <x86/drivers/include/io.h>
#include <x86/drivers/include/pic.h>
#include <kernel/include/interrupts.h>

#include <stdint.h>
#include <stddef.h>

extern uint32_t screen_line;

uint32_t Keyboard_HandleInterrupt() {
	Inb(0x60);

    char* screen = (char*)0xC00B8000;

	const char* str = "Keyboard interrupt received";
	uint8_t color = 0x0F;

    size_t i = 0;
    while (str[i] != 0) {
        screen[screen_line*160 + 2*i]   = str[i];
        screen[screen_line*160 + 2*i+1] = color;
        i++;
    }

    screen_line++;

	return 2;
}

void Keyboard_Initialize() {
	PIC_DisableInterrupt(KEYBOARD_PS2_IRQLINE);
	Interrupt_Handler_map[KEYBOARD_PS2_IRQLINE+PIC_IRQ_OFFSET].handler = &Keyboard_HandleInterrupt;
	PIC_EnableInterrupt(KEYBOARD_PS2_IRQLINE);
}
