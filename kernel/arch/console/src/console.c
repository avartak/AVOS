#include <kernel/arch/console/include/console.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/core/setup/include/setup.h>

uint16_t* Console_Screen = (uint16_t*)(CONSOLE_VGA_TEXT_BUFFER+KERNEL_HIGHER_HALF_OFFSET);
uint16_t  Console_pos = CONSOLE_VGA_NUM_COLUMNS;

void Console_MakeCursorInvisible() {

    X86_Outb(0x3D4, 0x0A);
    X86_Outb(0x3D5, 0x20);

}

void Console_MakeCursorVisible() {

    X86_Outb(0x3D4, 0x0A);
    X86_Outb(0x3D5, (X86_Inb(0x3D5) & 0xC0) | 0xE);
   
    X86_Outb(0x3D4, 0x0B);
    X86_Outb(0x3D5, (X86_Inb(0x3D5) & 0xE0) | 0xF);

}

void Console_SetCursorPosition(uint16_t pos) {

    X86_Outb(0x3D4, 0x0F);
    X86_Outb(0x3D5, (uint8_t)(pos & 0xFF));
    X86_Outb(0x3D4, 0x0E);
    X86_Outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));

}

uint16_t Console_Attribute(uint8_t fore, uint8_t back) {
	return ( (fore & 0x0F) + ((back & 0x0F) << 4) ) << 8;
}

void Console_ClearScreen() {
	for (uint32_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS * CONSOLE_VGA_NUM_LINES; i++) Console_Screen[i] = Console_Attribute(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK) | 0;
}

void Console_PrintWelcome() {

	Console_ClearScreen();

    for (size_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS; i++) Console_Screen[i] = Console_Attribute(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_GREEN) | 0;

    char* str = "Welcome to AVOS!";
    for (size_t i = 0; str[i] != 0; i++) Console_Screen[32+i] = Console_Attribute(CONSOLE_COLOR_RED, CONSOLE_COLOR_GREEN) | str[i];

	Console_SetCursorPosition(Console_pos);
	Console_MakeCursorVisible();
}

void Console_PrintChar(char c) {

	uint16_t color = Console_Attribute(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);

    if (c == 0) {
    }
    else if (c == '\n') {
        Console_pos = (Console_pos/CONSOLE_VGA_NUM_COLUMNS + 1)*CONSOLE_VGA_NUM_COLUMNS;
    }
    else if (c == '\t') {
		Console_pos += 3;
	}
    else if (c == '\b') {
		if (Console_pos == CONSOLE_VGA_NUM_COLUMNS) return;
        Console_pos--;
        Console_Screen[Console_pos] = color | 0;
    }
    else {
        Console_Screen[Console_pos++] = color | c;
    }
	Console_SetCursorPosition(Console_pos);
}

void Console_PrintChars(const char* string, uint32_t num) {

    for (uint32_t i = 0; i < num; i++) Console_PrintChar(string[i]);

}

void Console_PrintString(const char* string) {

    for (uint32_t i = 0; string[i] != 0; i++) Console_PrintChar(string[i]);

}

void Console_PrintNum(uint32_t num, bool hex) {

    char digits[]  = "0123456789ABCDEF";

    if (num < 10) {
        Console_PrintChar(digits[num]);
        return;
    }

    if (hex) {
        Console_PrintChar('0');
        Console_PrintChar('x');
    }

    bool start_printing = false;
    for (uint32_t divisor = (hex ? 0x10000000 : 1000000000); divisor > 0; divisor /= (hex ? 0x10 : 10)) {
        if (!start_printing && num/divisor > 0) start_printing = true;
        if (start_printing) Console_PrintChar(digits[num/divisor]);
        num %= divisor;
    }

}

