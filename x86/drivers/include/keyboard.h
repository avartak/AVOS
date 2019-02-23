#ifndef X86_DRIVERS_KEYBOARD_H
#define X86_DRIVERS_KEYBOARD_H

#include <stdint.h>

#define KEYBOARD_PS2_IRQLINE 1

extern uint32_t Keyboard_HandleInterrupt();
extern void     Keyboard_Initialize();

#endif
