#ifndef BOOT_CONSOLE_H
#define BOOT_CONSOLE_H

#include <boot/include/defs.h>

extern void Console_ClearScreen();
extern void Console_MakeCursorVisible();
extern void Console_MakeCursorInvisible();
extern void Console_SetCursorPosition(uint8_t line, uint8_t column);
extern void Console_PrintNum(uint32_t num, uint8_t line, uint8_t column, uint8_t color);
extern void Console_PrintChar(char c, uint8_t line, uint8_t column, uint8_t color);
extern void Console_PrintString(const char* string, uint8_t line, uint8_t column, uint8_t color);
extern char Console_ReadChar();

extern void Console_PrintBanner();
extern bool Console_PrintError(const char* string, bool retval);
extern void Console_ReadCommand(char* buffer);

#endif