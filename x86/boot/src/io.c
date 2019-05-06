#include <x86/boot/include/io.h>
#include <x86/boot/include/bios.h>

void IO_ClearScreen() {
    char* screen = (char*)0xB8000;

    size_t i = 0;
    while (i < 80*25) {
        screen[2*i]   = ' ';
        screen[2*i+1] = 0x0F;
        i++;
    }
}

void IO_PrintBanner() {

    char* screen = (char*)0xB8000;

    IO_ClearScreen();

    size_t i = 0;
    while (i < 80) {
        screen[2*i  ] = 0x00;
        screen[2*i+1] = 0x90;
        i++;
    }

    char* str = "AVOS Boot loader";
    i = 0;
    while (str[i] != 0) {
        screen[64 + 2*i]   = str[i];
        screen[64 + 2*i+1] = 0x9F;
        i++;
    }

    return;
}


void IO_PrintChar(char c, uint8_t line, uint8_t column, uint8_t color) {

    line = line % 25;
    column = column % 80;

    uint32_t pos = 2 * (line * 80 + column);

    char* screen = (char*)0xB8000;

    screen[pos]   = c;
    screen[pos+1] = color;

}

void IO_PrintString(const char* string, uint8_t line, uint8_t column, uint8_t color) {

    line = line % 25;
    column = column % 80;

    uint32_t pos = 2 * (line * 80 + column);

    char* screen = (char*)0xB8000;

	for (size_t i = 0; string[i] != 0; i++)	{
		screen[pos+2*i]   = string[i];
		screen[pos+2*i+1] = color;
	}
}

void IO_PrintNum(uint32_t num, uint8_t line, uint8_t column, uint8_t color) {

    line = line % 25;
    column = column % 80;

    uint32_t pos = 2 * (line * 80 + column);

    char* screen = (char*)0xB8000;

    screen[pos] = '0';
    pos++;
    screen[pos] = color;
    pos++;

    screen[pos] = 'x';
    pos++;
    screen[pos] = color;
    pos++;

    uint32_t divisor = 0x10000000;
    uint32_t digit = 0;

    for (size_t i = 0; i < 8; i++) {

        digit = num / divisor;

        uint8_t cdigit = (uint8_t)digit;

        if (digit < 0xA) cdigit += 0x30;
        else cdigit += (0x41 - 0xA);
        screen[pos] = cdigit;
        pos++;
        screen[pos] = color;
        pos++;

        num = num % divisor;
        divisor /= 0x10;

    }

}

void IO_SetCursorPosition(uint8_t line, uint8_t column) {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax = 0x0200;
    BIOS_regs.edx = (line << 8) + column;

    BIOS_Interrupt(0x10, &BIOS_regs);
}

char IO_ReadChar() {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax = 0;

    BIOS_Interrupt(0x16, &BIOS_regs);
    if ((BIOS_regs.flags & 1) == 1) return 0;
	else return BIOS_regs.eax & 0xFF;

}

void IO_MakeCursorInvisible() {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.ecx = 0x2607;
    BIOS_regs.eax = 0x0100;

    BIOS_Interrupt(0x10, &BIOS_regs);

}

void IO_MakeCursorVisible() {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.ecx = 0x0607;
    BIOS_regs.eax = 0x0100;

    BIOS_Interrupt(0x10, &BIOS_regs);

}

void IO_ReadCommand(char* buffer) {

	uint8_t start_line   = 2;
	uint8_t start_column = 0;

	uint8_t end_line     = 14;
	uint8_t end_column   = 70;

	uint8_t line         = start_line;
	uint8_t column       = start_column;
	uint8_t color        = 0x0F;

	size_t  pos          = 0;

	IO_MakeCursorVisible();
	
	IO_PrintChar('[', line, column++, color);
	IO_PrintChar('A', line, column++, color);
	IO_PrintChar('V', line, column++, color);
	IO_PrintChar('B', line, column++, color);
	IO_PrintChar('L', line, column++, color);
	IO_PrintChar(':', line, column++, color);
	IO_PrintChar(']', line, column++, color);

	while (true) {
		IO_SetCursorPosition(line, column);
		char c = IO_ReadChar();
		if (c >= 0x20 && c < 0x7F) {
			IO_PrintChar(c, line, column, color);
			if (column == 79) {
				column = 0;
				line++;
			}
			else column++;
			buffer[pos++] = c;
		}
		if (c == 0x0D) {
			buffer[pos] = '\0';
			break;
		}
		if (c == 8) {
			if (line == start_line && column == start_column+7) continue;
			else if (column == 0) {
				column = 79;
				line--;
			}
			else column--;
			IO_PrintChar(' ', line, column, color);
			pos--;
		}

		if (line == end_line && column == end_column) {
			IO_PrintString("Command line buffer full. Press enter to continue", 23, 0, 0x04);
			while (IO_ReadChar() != 0x0D) {
			}
			buffer[pos] = '\0';
			break;
		}
	}
}
