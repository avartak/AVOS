#include <x86/drivers/include/keyboard.h>
#include <x86/drivers/include/io.h>
#include <x86/drivers/include/pic.h>
#include <kernel/include/interrupts.h>
#include <kernel/include/heap.h>

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
    struct Interrupt_Handler* handler = (struct Interrupt_Handler*)(KHeap_Allocate(sizeof(struct Interrupt_Handler)));
    handler->next     = MEMORY_NULL_PTR;
    handler->handler  = &Keyboard_HandleInterrupt;
    handler->id       = 0;
    handler->process  = 0;

    uint8_t retval = Interrupt_AddHandler(handler, 0x21);
    if (retval == 0 || retval == 1) KHeap_Free((uintptr_t)handler);
}
