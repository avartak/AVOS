#include <x86/kernel/include/welcome.h>

void Welcome() {

    // Video buffer -- now at 0xC00B8000
    char* screen = (char*)0xC00B8000;

    unsigned int i = 0;

    // Clear screen
    while (i < 80*25*2) {
        screen[i] = 0;
        i++;
    }

    // Welcome string
    char* str = "Welcome to AVOS!";


    // Print message to screen -- the magic number 64 causes the message to start on line 0 and 32 character offset
    i = 0;
    while (str[i] != 0) {
        screen[64 + 2*i]   = str[i];
        screen[64 + 2*i+1] = 0x04;
        i++;
    }

    return;

}
