#ifndef KERNEL_CONSOlE_H
#define KERNEL_CONSOlE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

uint16_t* Console_Screen;
uint16_t  Console_pos;

extern void Console_PrintWelcome();
extern void Console_PrintNum(uint32_t num, bool hex);
extern void Console_PrintChar(char c);
extern void Console_PrintChars(const char* string, uint32_t num);
extern void Console_PrintString(const char* string);

#endif
