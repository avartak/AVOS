#include <boot/x86/BIOS/include/console.h>
#include <boot/x86/BIOS/include/bios.h>

// Clear screen
void Console_ClearScreen() {
    char* screen = (char*)0xB8000;

    size_t i = 0;
    while (i < 80*25) {
        screen[2*i]   = ' ';
        screen[2*i+1] = 0x0F;
        i++;
    }
}

// Make the cursor invisible (no more blinking dash)
void Console_MakeCursorInvisible() {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.ecx = 0x2607;
    BIOS_regs.eax = 0x0100;

    BIOS_Interrupt(0x10, &BIOS_regs);

}

// Make the cursor visible
void Console_MakeCursorVisible() {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.ecx = 0x0607;
    BIOS_regs.eax = 0x0100;

    BIOS_Interrupt(0x10, &BIOS_regs);

}

// Place the cursor at a certain location on screen
void Console_SetCursorPosition(uint8_t line, uint8_t column) {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax = 0x0200;
    BIOS_regs.edx = (line << 8) + column;

    BIOS_Interrupt(0x10, &BIOS_regs);
}

// Print a character at a certain location on screen
void Console_PrintChar(char c, uint8_t line, uint8_t column, uint8_t color) {

    line = line % 25;
    column = column % 80;

    uint32_t pos = 2 * (line * 80 + column);

    char* screen = (char*)0xB8000;

    screen[pos]   = c;
    screen[pos+1] = color;

}

// Print a character starting at a certain location on screen
void Console_PrintString(const char* string, uint8_t line, uint8_t column, uint8_t color) {

    line = line % 25;
    column = column % 80;

    uint32_t pos = 2 * (line * 80 + column);

    char* screen = (char*)0xB8000;

	for (size_t i = 0; string[i] != 0; i++)	{
		screen[pos+2*i]   = string[i];
		screen[pos+2*i+1] = color;
	}
}

// Print a 32-bit unsigned integer in hex format at a certain location on screen
void Console_PrintNum(uint32_t num, uint8_t line, uint8_t column, uint8_t color) {

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

// Read a character from the keyboard
char Console_ReadChar() {

    struct BIOS_Registers BIOS_regs;
    BIOS_ClearRegistry(&BIOS_regs);

    BIOS_regs.eax = 0;

    BIOS_Interrupt(0x16, &BIOS_regs);
    if ((BIOS_regs.flags & 1) == 1) return 0;
	else return BIOS_regs.eax & 0xFF;

}

// Print the banner for the 'AVBL' bootloader 
void Console_PrintBanner() {

    char* screen = (char*)0xB8000;

    Console_ClearScreen();

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

	Console_MakeCursorInvisible();

    return;
}

// Print message passed as an argument, and returns the boolean value also passed as argument
bool Console_PrintError(const char* string, bool retval) {

	Console_PrintString(string, 23, 0, 4);
	return retval;
}

// Read a command string from the keyboard and print it on the screen
// Input ends when enter is pressed (ASCII code 0x0D)
// At most 0x400 characters allowed
void Console_ReadCommand(char* buffer) {

	uint8_t color        = 0x0F;

	uint8_t start_line   = 2;
	uint8_t start_column = 0;

	uint8_t line         = start_line;
	uint8_t column       = start_column;

	Console_MakeCursorVisible();
	Console_PrintChar('[', line, column++, color);
	Console_PrintChar('A', line, column++, color);
	Console_PrintChar('V', line, column++, color);
	Console_PrintChar('B', line, column++, color);
	Console_PrintChar('L', line, column++, color);
	Console_PrintChar(':', line, column++, color);
	Console_PrintChar(']', line, column++, color);

	size_t pos = line*80 + column + 0x3FF;
	uint8_t end_line   = pos / 80;
	uint8_t end_column = pos % 80;

	pos = 0;

	while (true) {
		// Set the cursor position on the current line/column
		Console_SetCursorPosition(line, column);

		// Read character from keyboard
		char c = Console_ReadChar();

		// If the character is alphanumeric print it at the current location and advance the line/column
		if (c >= 0x20 && c < 0x7F) {
			Console_PrintChar(c, line, column, color);
			if (column == 79) {
				column = 0;
				line++;
			}
			else column++;
			buffer[pos++] = c;
		}

		// If an enter key was pressed then null-terminate the input buffer and end
		if (c == 0x0D) {
			buffer[pos] = '\0';
			break;
		}

		// If a backspace was pressed then delete the characted at the current position (replace with a space) and move back the current location by one
		if (c == 8) {
			if (line == start_line && column == start_column+7) continue;
			else if (column == 0) {
				column = 79;
				line--;
			}
			else column--;
			Console_PrintChar(' ', line, column, color);
			pos--;
		}

		// If the input buffer is full (0x400 characters read) then issue a message that the buffer is full and ask the user to press enter to continue
		if (line == end_line && column == end_column) {
			Console_PrintString("Command line buffer full. Press enter to continue", 23, 0, 0x04);
			while (Console_ReadChar() != 0x0D) {
			}
			buffer[pos] = '\0';
			break;
		}
	}
}
