#include <stddef.h>

void Welcome() {

    // Video buffer
    char* screen = (char*)0xC00B8000;

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

