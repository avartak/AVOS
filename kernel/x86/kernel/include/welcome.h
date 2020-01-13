#ifndef X86_KERNEL_WELCOME_H
#define X86_KERNEL_WELCOME_H

#include <kernel/include/common.h>

/*

Write a welcome message directly to the vide buffer
Tells us things are working well

*/

extern uint32_t screen_line;

extern void Welcome();
extern void ClearScreen();
extern void PrintNum(uint32_t num, uint8_t line, uint8_t column);

#endif
