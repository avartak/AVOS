#include <x86/kernel/include/welcome.h>

#include <stdint.h>
#include <stddef.h>

void Welcome() {

    // Video buffer -- now at 0xC00B8000
    char* screen = (char*)0xC00B8000;

    size_t i = 0;

    // Clear screen
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

	uint32_t* memory_map = (uint32_t*)0xC0010000;
	uint32_t  map_size   = memory_map[0];
	memory_map++;

	uint8_t line = 0;
	uint8_t col  = 0;
	uint8_t map_pos = 0;
	for (size_t i = 0; i < map_size; i++) {
		line++;
		col = 0;
		PrintNum(memory_map[map_pos], line, col);
		col += 0x10;
		map_pos += 2;
		PrintNum(memory_map[map_pos], line, col);
		col += 0x10;
		map_pos += 2;
		PrintNum(memory_map[map_pos], line, col);
		col += 0x10;
		map_pos += 1;
		PrintNum(memory_map[map_pos], line, col);
		map_pos += 1;
	}

    return;

}


void PrintNum(uint32_t num, uint8_t line, uint8_t column) {

	line = line % 25;
	column = column % 80;	

	uint32_t pos = 2 * (line * 80 + column);

	char* screen = (char*)0xC00B8000;

	screen[pos] = '0';
	pos++;
	screen[pos] = 0x0F;
	pos++;

	screen[pos] = 'x';
	pos++;
	screen[pos] = 0x0F;
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
		screen[pos] = 0x0F;
		pos++;  

		num = num % divisor;
		divisor /= 0x10;

	}

}
