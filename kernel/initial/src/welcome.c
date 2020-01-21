#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/initial/include/setup.h>
#include <kernel/arch/i386/include/ioports.h>

void Welcome() {

    // Video buffer
    char* screen = (char*)(0xB8000+KERNEL_HIGHER_HALF_OFFSET);

	// Make the cursoor invisible
	X86_Outb(0x3D4, 0x0A);
	X86_Outb(0x3D5, 0x20);

    // Clear screen
    size_t i = 0;
    while (i < 80*25*2) {
        screen[i] = 0;
        i++;
    }

    // Green stripe
	i = 0;		
    while (i < 80) {
        screen[2*i  ] = 0x00;
        screen[2*i+1] = 0x20;
        i++;
    }

    // Welcome string
    char* str = "Welcome to AVOS!";


    // Print message to screen -- the magic number 64 causes the message to start on line 0 and 32 character offset
    i = 0;
    while (str[i] != 0) {
        screen[64 + 2*i]   = str[i];
        screen[64 + 2*i+1] = 0x24;
        i++;
    }

    return;

}

void PrintNum(uint32_t num, uint8_t line, uint8_t column) {

    uint32_t pos   = line * 80 + column;
    uint16_t color = 0x0F00;
    char digits[]  = "0123456789ABCDEF";

    uint16_t* screen = (uint16_t*)(0xB8000+KERNEL_HIGHER_HALF_OFFSET);

    if (num < 10) {
		screen[pos] = color | digits[num];
		return;
	}

    screen[pos++] = color | '0';
    screen[pos++] = color | 'x';

    bool start_printing = false;
    for (uint32_t divisor = 0x10000000; divisor > 0; divisor /= 0x10) {
        if (!start_printing && num/divisor > 0) start_printing = true;
        if (start_printing) screen[pos++] = color | digits[num/divisor];
        num %= divisor;
    }

}

void PrintChar(char c, uint8_t line, uint8_t column) {

    uint16_t* screen = (uint16_t*)(0xB8000+KERNEL_HIGHER_HALF_OFFSET);
    uint32_t pos = line * 80 + column;

    screen[pos] = 0x0F00 | c;

}

void PrintChars(const char* string, uint32_t num, uint8_t line, uint8_t column) {

    uint16_t* screen = (uint16_t*)(0xB8000+KERNEL_HIGHER_HALF_OFFSET);
    uint32_t pos = line * 80 + column;

    for (uint32_t i = 0; i < num; i++) screen[pos+i] = 0x0F00 | string[i];

}

void PrintString(const char* string, uint8_t line, uint8_t column) {

    uint16_t* screen = (uint16_t*)(0xB8000+KERNEL_HIGHER_HALF_OFFSET);
    uint32_t pos = line * 80 + column;

    for (uint32_t i = 0; string[i] != 0; i++) screen[pos+i] = 0x0F00 | string[i];

}


