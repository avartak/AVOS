/*

These functions are used for text I/O if needed in the bootloader

Several functions that are used to display characters on screen write directly to the linear text buffer located at 0xB8000
- It is assumed that the video is set to 80x25 text mode by BIOS
- This is not explicitly enforced as of now, but could be done (e.g. by setting the VGA mode to 3 or something equivalent in the Console_PrintBanner function) 

* Console_ClearScreen         : Clears the contents of the screen to show a complete blank
* Console_MakeCursorVisible   : Show the default cursor (underline, blinking)
* Console_MakeCursorInvisible : Hide the cursor
* Console_SetCursorPosition   : Set the cursor position to a given line & column
* Console_PrintNum            : Print 32-bit unsigned integer to a given line & column on screen with given color attribute; option to print in hex or decimal
* Console_PrintChar           : Print a character to a given line & column on screen with given color attribute
* Console_PrintString         : Print a 0-terminated string to screen starting at a given line & column with given color attribute
* Console_ReadChar            : Read a character from keyboard using INT 0x16, AH=0x00 ; the read character is stored in AL

* Console_PrintBanner         : First clears the screen, then prints the AVBL banner on the first line, and makes the cursor invisible
* Console_PrintAndReturn      : Prints an error string on line 23 in red color and returns the boolean passed as the second argument (useful when returning a boolean status + message)
* Console_ReadCommand         : Reads a command string from keyboard (up to 0x400 characters) and displays it at the AVBL command prompt

Note: 
Code for cursor manipulation based on direct I/O with the VGA controller is taken from:
https://wiki.osdev.org/Text_Mode_Cursor

*/

#ifndef BOOTLOADER_CONSOLE_H
#define BOOTLOADER_CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

#define CONSOLE_VGA_TEXT_BUFFER    0xB8000
#define CONSOLE_VGA_NUM_LINES      25
#define CONSOLE_VGA_NUM_COLUMNS    80
#define CONSOLE_KEY_ENTER          0x0D
#define CONSOLE_KEY_SPACE          0x20

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

extern uint16_t Console_Attribute(uint8_t fore, uint8_t back);
extern void     Console_ClearScreen();
extern void     Console_MakeCursorVisible();
extern void     Console_MakeCursorInvisible();
extern void     Console_SetCursorPosition(uint8_t line, uint8_t column);
extern void     Console_PrintNum(uint32_t num, bool hex, uint8_t line, uint8_t column, uint8_t fore_color, uint8_t back_color);
extern void     Console_PrintChar(char c, uint8_t line, uint8_t column, uint8_t fore_color, uint8_t back_color);
extern void     Console_PrintChars(const char* string, uint32_t num, uint8_t line, uint8_t column, uint8_t fore_color, uint8_t back_color);
extern void     Console_PrintString(const char* string, uint8_t line, uint8_t column, uint8_t fore_color, uint8_t back_color);
extern char     Console_ReadChar();

extern void     Console_PrintBanner();
extern bool     Console_PrintAndReturn(const char* string, bool retval);
extern void     Console_ReadCommand(char* buffer);

#endif
