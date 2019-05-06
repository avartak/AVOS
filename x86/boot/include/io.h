#ifndef X86_BOOT_IO_H
#define X86_BOOT_IO_H

#include <kernel/include/common.h>

extern void IO_ClearScreen();
extern void IO_MakeCursorVisible();
extern void IO_MakeCursorInvisible();
extern void IO_PrintBanner();
extern void IO_PrintNum(uint32_t num, uint8_t line, uint8_t column, uint8_t color);
extern void IO_PrintChar(char c, uint8_t line, uint8_t column, uint8_t color);
extern void IO_PrintString(const char* string, uint8_t line, uint8_t column, uint8_t color);
extern void IO_SetCursorPosition(uint8_t line, uint8_t column);
extern char IO_ReadChar();
extern void IO_ReadCommand(char* buffer);

#endif
