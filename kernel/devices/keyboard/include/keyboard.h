#ifndef KERNEL_KEYBOARD_H
#define KERNEL_KEYBOARD_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/arch/tasking/include/context.h>

#define KEYBOARD_PS2_IRQLINE 1

// PC keyboard interface constants

#define KEYBOARD_STAT_PORT      0x64    // kbd controller status port(I)
#define KEYBOARD_DATA_PORT      0x60    // kbd data port(I)
#define KEYBOARD_DATA_IN_BUFFER 0x01    // kbd data in buffer

#define NO                      0

#define KEY_SHIFT               (1<<0)
#define KEY_CTL                 (1<<1)
#define KEY_ALT                 (1<<2)
#define KEY_CAPSLOCK            (1<<3)
#define KEY_NUMLOCK             (1<<4)
#define KEY_SCROLLLOCK          (1<<5)
#define KEY_E0ESC               (1<<6)

// Special keycodes
#define KEY_HOME                0xE0
#define KEY_END                 0xE1
#define KEY_UP                  0xE2
#define KEY_DN                  0xE3
#define KEY_LF                  0xE4
#define KEY_RT                  0xE5
#define KEY_PGUP                0xE6
#define KEY_PGDN                0xE7
#define KEY_INS                 0xE8
#define KEY_DEL                 0xE9

// KEY_C('A') == Control-A
#define C(x)                  (x - '@')

extern uint32_t Keyboard_screen_pos;
extern uint8_t  Keyboard_GetChar();
extern void     Keyboard_HandleInterrupt(struct IContext* frame);
extern void     Keyboard_Initialize(uint8_t irq, uint8_t vector);

#endif
