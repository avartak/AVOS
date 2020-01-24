#include <kernel/arch/console/include/console.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/core/setup/include/setup.h>

uint16_t* Console_Screen = (uint16_t*)(0xB8000+KERNEL_HIGHER_HALF_OFFSET);
uint16_t  Console_pos = 80;

void Console_PrintWelcome() {

    // Make the cursoor invisible
    X86_Outb(0x3D4, 0x0A);
    X86_Outb(0x3D5, 0x20);

    // Clear screen
    size_t i = 0;
    while (i < 80*25) Console_Screen[i++] = 0;

    // Green stripe
    i = 0;
    while (i < 80) Console_Screen[i++] = 0x2000;

    // Welcome string
    char* str = "Welcome to AVOS!";


    // Print message to screen -- the magic number 64 causes the message to start on line 0 and 32 character offset
    i = 0;
    while (str[i] != 0) {
		Console_Screen[32+i]   = 0x2400 | str[i];
		i++;
	}
}


void Console_PrintNum(uint32_t num, bool hex) {

	uint16_t color = 0x0F00;
	char digits[]  = "0123456789ABCDEF";
	
	if (num < 10) {
		Console_Screen[Console_pos++] = color | digits[num];
		return;
	}

	if (hex) {
		Console_Screen[Console_pos++] = color | '0';
		Console_Screen[Console_pos++] = color | 'x';
	}
	
	bool start_printing = false;
	for (uint32_t divisor = (hex ? 0x10000000 : 1000000000); divisor > 0; divisor /= (hex ? 0x10 : 10)) {
		if (!start_printing && num/divisor > 0) start_printing = true;
		if (start_printing) Console_Screen[Console_pos++] = color | digits[num/divisor];
		num %= divisor;
	}

}

void Console_PrintChar(char c) {

    if (c == 0) {
    }
    else if (c == '\n') {
        Console_pos = (Console_pos/80 + 1)*80;
    }
    else if (c == 8) {
        Console_pos--;
        Console_Screen[Console_pos] = 0x0F00;
    }
    else {
        Console_Screen[Console_pos++] = 0x0F00 | c;
    }

}

void Console_PrintChars(const char* string, uint32_t num) {

    for (uint32_t i = 0; i < num; i++) Console_PrintChar(string[i]);

}

void Console_PrintString(const char* string) {

    for (uint32_t i = 0; string[i] != 0; i++) Console_PrintChar(string[i]);

}


