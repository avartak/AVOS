#ifndef KERNEL_CONSOlE_H
#define KERNEL_CONSOlE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CONSOLE_VGA_TEXT_BUFFER    0xB8000
#define CONSOLE_VGA_NUM_LINES      25
#define CONSOLE_VGA_NUM_COLUMNS    80

#define CONSOLE_POS_START_WELCOME  32
#define CONSOLE_POS_START          80
#define CONSOLE_POS_END            80*25

#define CONSOLE_COLOR_BLACK        0
#define CONSOLE_COLOR_BLUE         1
#define CONSOLE_COLOR_GREEN        2
#define CONSOLE_COLOR_CYAN         3
#define CONSOLE_COLOR_RED          4
#define CONSOLE_COLOR_MAGENTA      5
#define CONSOLE_COLOR_BROWN        6
#define CONSOLE_COLOR_LIGHT_GREY   7
#define CONSOLE_COLOR_DARK_GREY    8
#define CONSOLE_COLOR_LIGHT_BLUE   9
#define CONSOLE_COLOR_LIGHT_GREEN  0xA
#define CONSOLE_COLOR_LIGHT_CYAN   0xB
#define CONSOLE_COLOR_ORANGE       0xC
#define CONSOLE_COLOR_PINK         0xD
#define CONSOLE_COLOR_LIGHT_BROWN  0xE
#define CONSOLE_COLOR_WHITE        0xF

uint16_t* Console_Screen;
uint16_t  Console_pos;

extern uint16_t Console_Attribute(uint8_t fore, uint8_t back);
extern void     Console_ClearScreen();
extern void     Console_MakeCursorVisible();
extern void     Console_MakeCursorInvisible();
extern void     Console_SetCursorPosition(uint16_t pos);

extern void     Console_PrintWelcome();
extern void     Console_PrintNum(uint32_t num, bool hex);
extern void     Console_PrintChar(char c);
extern void     Console_PrintChars(const char* string, uint32_t num);
extern void     Console_PrintString(const char* string);

#endif
