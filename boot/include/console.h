/*

These functions are used for text I/O if needed in the bootloader

Several functions that are used to display characters on screen write directly to the linear text buffer located at 0xB8000
- It is assumed that the video is set to 80x25 text mode by BIOS
- This is not explicitly enforced as of now, but could be done (e.g. by setting the VGA mode to 3 or something equivalent in the Console_PrintBanner function) 

* Console_ClearScreen         : Clears the contents of the screen to show a complete blank
* Console_MakeCursorVisible   : Show the default cursor (underline, blinking) using INT 0x10, AH=0x01 ; achieved by setting CX=0x0607 when calling the BIOS routine
* Console_MakeCursorInvisible : Hide the using INT 0x10, AH=0x01 ; achieved by setting CX=0x2607 when calling the BIOS routine
* Console_SetCursorPosition   : Set the cursor position to a given line & column using INT 0x10, AH=0x02 ; set DH=row, DL=column, BH=0 (page number)
* Console_PrintNum            : Print 32-bit unsigned integer to a given line & column on screen with given color attribute; number starts with 0x and always shows 8 numerals (0-padded)
* Console_PrintChar           : Print a character to a given line & column on screen with given color attribute
* Console_PrintString         : Print a 0-terminated string to screen starting at a given line & column with given color attribute
* Console_ReadChar            : Read a character from keyboard using INT 0x16, AH=0x00 ; the read character is stored in AL

* Console_PrintBanner         : First clears the screen, then prints the AVBL banner on the first line, and makes the cursor invisible
* Console_PrintError          : Prints an error string on line 23 in red color and returns the boolean passed as the second argument (useful when returning a boolean status + message)
* Console_ReadCommand         : Reads a command string from keyboard (up to 0x400 characters) and displays it at the AVBL command prompt

*/

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
