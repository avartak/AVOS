#include <boot/include/console.h>
#include <boot/include/bios.h>
#include <boot/include/ioports.h>

uint16_t* Console_Screen = (uint16_t*)CONSOLE_VGA_TEXT_BUFFER;

// Construct the attribute byte from foreground and background colors
uint16_t Console_Attribute(uint8_t fore, uint8_t back) {

	return ( (fore & 0x0F) + ((back & 0x0F) << 4) ) << 8;
}

// Clear screen
void Console_ClearScreen() {
	
	for (uint32_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS * CONSOLE_VGA_NUM_LINES; i++) Console_Screen[i] = Console_Attribute(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);

}

// Make the cursor invisible (no more blinking dash)
void Console_MakeCursorInvisible() {

	IOPorts_Outb(0x3D4, 0x0A);
	IOPorts_Outb(0x3D5, 0x20);

}

// Make the cursor visible
void Console_MakeCursorVisible() {

	IOPorts_Outb(0x3D4, 0x0A);
	IOPorts_Outb(0x3D5, (IOPorts_Inb(0x3D5) & 0xC0) | 0xE);
	
	IOPorts_Outb(0x3D4, 0x0B);
	IOPorts_Outb(0x3D5, (IOPorts_Inb(0x3D5) & 0xE0) | 0xF);

}

// Place the cursor at a certain location on screen
void Console_SetCursorPosition(uint8_t line, uint8_t column) {

	uint16_t pos = line * CONSOLE_VGA_NUM_COLUMNS + column;
	
	IOPorts_Outb(0x3D4, 0x0F);
	IOPorts_Outb(0x3D5, (uint8_t)(pos & 0xFF));
	IOPorts_Outb(0x3D4, 0x0E);
	IOPorts_Outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));

}

// Print a character at a certain location on screen
void Console_PrintChar(char c, uint8_t line, uint8_t column, uint8_t fore_color, uint8_t back_color) {

	uint32_t pos = line * CONSOLE_VGA_NUM_COLUMNS + column;
	Console_Screen[pos] = Console_Attribute(fore_color, back_color) | c;

}

// Print a character starting at a certain location on screen
void Console_PrintString(const char* string, uint8_t line, uint8_t column, uint8_t fore_color, uint8_t back_color) {

	uint32_t pos = line * CONSOLE_VGA_NUM_COLUMNS + column;
	for (uint32_t i = 0; string[i] != 0; i++) Console_Screen[pos+i] = Console_Attribute(fore_color, back_color) | string[i]; 

}

// Print a 32-bit unsigned integer in hex format at a certain location on screen
void Console_PrintNum(uint32_t num, bool hex, uint8_t line, uint8_t column, uint8_t fore_color, uint8_t back_color) {

	uint32_t pos   = line * CONSOLE_VGA_NUM_COLUMNS + column;
	uint16_t color = Console_Attribute(fore_color, back_color);
	char digits[]  = "0123456789ABCDEF";
	
	if (hex) {
		Console_Screen[pos++] = color | '0';	
		Console_Screen[pos++] = color | 'x';	
	}	
	if (num == 0) Console_Screen[pos++] = color | '0';

	bool start_printing = false;
	for (uint32_t divisor = (hex ? 0x10000000 : 1000000000); divisor > 0; divisor /= (hex ? 0x10 : 10)) {
		if (!start_printing && num/divisor > 0) start_printing = true;
		if (start_printing) Console_Screen[pos++] = color | digits[num/divisor];	
		num %= divisor;
	}

}

// Read a character from the keyboard
char Console_ReadChar() {

	struct BIOS_Registers BIOS_regs;
	BIOS_ClearRegistry(&BIOS_regs);
	
	BIOS_Interrupt(0x16, &BIOS_regs);
	if (BIOS_regs.flags & 1) return 0;
	else return BIOS_regs.eax & 0xFF;

}

// Print the banner for the 'AVBL' bootloader 
void Console_PrintBanner() {

	char* str = "AVOS Boot loader";

	Console_ClearScreen();
	for (uint32_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS; i++) Console_Screen[i] = Console_Attribute(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_LIGHT_BLUE);
	for (uint32_t i = 0; str[i] != 0; i++) Console_Screen[32+i] = Console_Attribute(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_LIGHT_BLUE) + str[i];
	Console_MakeCursorInvisible();
}

// Print message passed as an argument, and returns the boolean value also passed as argument
bool Console_PrintAndReturn(const char* string, bool retval) {

	Console_PrintString(string, 23, 0, CONSOLE_COLOR_RED, CONSOLE_COLOR_BLACK);
	return retval;
}

// Read a command string from the keyboard and print it on the screen
// Input ends when enter is pressed (ASCII code 0x0D)
// At most 0x400 characters allowed
void Console_ReadCommand(char* buffer) {

	uint8_t line   = 2;
	uint8_t column = 0;
	
	Console_MakeCursorVisible();
	Console_PrintString("[AVBL:]", line, column, CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);	

	uint8_t  start_line   = line;
	uint8_t  start_column = (column += 7);
	uint32_t pos          = line*CONSOLE_VGA_NUM_COLUMNS + column + 0x3FF;
	uint8_t  end_line     = pos / CONSOLE_VGA_NUM_COLUMNS;
	uint8_t  end_column   = pos % CONSOLE_VGA_NUM_COLUMNS;
		
	pos = 0;
	while (true) {
		// Set the cursor position on the current line/column
		Console_SetCursorPosition(line, column);
		
		// Read character from keyboard
		char c = Console_ReadChar();
		
		// If the character is alphanumeric print it at the current location and advance the line/column
		if (c >= CONSOLE_KEY_SPACE && c < 0x7F) {
			Console_PrintChar(c, line, column, CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);
			if (column == CONSOLE_VGA_NUM_COLUMNS - 1) {
				column = 0;
				line++;
			}
			else column++;
			buffer[pos++] = c;
		}
		
		// If a backspace was pressed then delete the characted at the current position (replace with a space) and move back the current location by one
		if (c == 8) {
			if (line == start_line && column == start_column) continue;
			else if (column == 0) {
				column = CONSOLE_VGA_NUM_COLUMNS - 1;
				line--;
			}
			else column--;
			Console_PrintChar(' ', line, column, CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);
			pos--;
		}
		
		// If an enter key was pressed then null-terminate the input buffer and end
		if (c == CONSOLE_KEY_ENTER) {
			buffer[pos] = '\0';
			break;
		}
		
		// If the input buffer is full (0x400 characters read) then issue a message that the buffer is full and ask the user to press enter to continue
		if (line == end_line && column == end_column) {
			Console_PrintString("Command line buffer full. Press enter to continue", 23, 0, CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);
			while (Console_ReadChar() != CONSOLE_KEY_ENTER) {}
			buffer[pos] = '\0';
			break;
		}
	}
}
