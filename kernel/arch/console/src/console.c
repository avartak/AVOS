#include <kernel/arch/console/include/console.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/clib/include/string.h>

uint16_t* Console_screen = (uint16_t*)(CONSOLE_VGA_TEXT_BUFFER+KERNEL_HIGHER_HALF_OFFSET);
uint16_t  Console_pos = CONSOLE_POS_START;
bool      Console_inpanic = false;

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

void Console_ClearLine(uint8_t line) {
	for (size_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS; i++) {
		Console_screen[line*CONSOLE_VGA_NUM_COLUMNS+i] = Console_Attribute(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK) | 0;
	}
}

void Console_ClearScreen() {
	for (size_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS * CONSOLE_VGA_NUM_LINES; i++) Console_screen[i] = Console_Attribute(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK) | 0;
	Console_ClearLine(CONSOLE_VGA_NUM_LINES);
}

void Console_PrintWelcome() {

	Console_ClearScreen();

    for (size_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS; i++) Console_screen[i] = Console_Attribute(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_GREEN) | 0;

    char* str = "Welcome to AVOS!";
    for (size_t i = 0; str[i] != 0; i++) Console_screen[CONSOLE_POS_START_WELCOME+i] = Console_Attribute(CONSOLE_COLOR_RED, CONSOLE_COLOR_GREEN) | str[i];

	Console_SetCursorPosition(Console_pos);
	Console_MakeCursorVisible();

}

void Console_PrintChar(char c) {

	if (Console_inpanic) {
		while (true);
	}

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
		if (Console_pos == CONSOLE_POS_START) return;
        Console_pos--;
        Console_screen[Console_pos] = color | 0;
    }
    else {
        Console_screen[Console_pos++] = color | c;
    }

	if (Console_pos >= CONSOLE_VGA_NUM_COLUMNS * CONSOLE_VGA_NUM_LINES) {
		memmove(&Console_screen[CONSOLE_POS_START], &Console_screen[CONSOLE_POS_START+CONSOLE_VGA_NUM_COLUMNS], (Console_pos-CONSOLE_POS_START)*2);
		Console_pos -= CONSOLE_VGA_NUM_COLUMNS;
	}
	Console_ClearLine(CONSOLE_VGA_NUM_LINES);
	Console_SetCursorPosition(Console_pos);
}

void Console_PrintChars(const char* string, uint32_t num) {

    for (uint32_t i = 0; i < num; i++) Console_PrintChar(string[i]);

}

void Console_PrintString(const char* string) {

    for (uint32_t i = 0; string[i] != 0; i++) Console_PrintChar(string[i]);

}

void Console_PrintNum(uint32_t num, bool hex, bool withsign) {

    char digits[]  = "0123456789ABCDEF";
	bool neg = num > 0x80000000;
	uint32_t val = num;
	if (withsign) val = (neg ? 0-num : num);

	if (withsign && neg) Console_PrintChar('-');

    if (val < 10) {
        Console_PrintChar(digits[val]);
        return;
    }

    if (hex) {
        Console_PrintChar('0');
        Console_PrintChar('x');
    }

    bool start_printing = false;
    for (uint32_t divisor = (hex ? 0x10000000 : 1000000000); divisor > 0; divisor /= (hex ? 0x10 : 10)) {
        if (!start_printing && val/divisor > 0) start_printing = true;
        if (start_printing) Console_PrintChar(digits[val/divisor]);
        val %= divisor;
    }

}

void Console_Print(const char* format, ...) {

	va_list args;
	va_list args_copy;
	va_start(args, format);
	va_copy(args_copy, args);
	va_end(args);
	Console_VPrint(format, args_copy);
	va_end(args_copy);

}

void Console_VPrint(const char* format, va_list args) {

	char c = *format;
	while (c != '\0') {
		if (c != '%') {
			Console_PrintChar(c);
			c = *(++format);
			continue;
		}
		c = *(++format);
		if (c == '\0') break;
		else if (c == 'c') {
			char ch = va_arg(args, int);
			Console_PrintChar(ch);
		}
		else if (c == 'd' || c == 'i') {
			uint32_t num = va_arg(args, uint32_t);
			Console_PrintNum(num, false, true);
		}
		else if (c == 'u') {
			uint32_t num = va_arg(args, uint32_t);
			Console_PrintNum(num, false, false);
		}
		else if (c == 'x') {
			uint32_t num = va_arg(args, uint32_t);
			Console_PrintNum(num, true, false);
		}
		else if (c == 'p') {
			uint32_t num = va_arg(args, uint32_t);
			Console_PrintNum(num, true, false);
		}
		else if (c == 's') {
			char* str = va_arg(args, char*);
			Console_PrintString(str);
		}
		else if (c == '%') {
			Console_PrintChar(c);
		}
		else {
			Console_PrintChar('%');
			Console_PrintChar(c);
		}
		c = *(++format);
	}

}

void Console_Panic(const char* format, ...) {

    va_list args;
    va_list args_copy;
    va_start(args, format);
    va_copy(args_copy, args);
    va_end(args);
    Console_VPrint(format, args_copy);
    va_end(args_copy);

	Console_inpanic = true;
	while (true);	
}
