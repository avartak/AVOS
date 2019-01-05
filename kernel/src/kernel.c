#include <drivers/include/memory.h>
#include <drivers/include/console.h>

void kmain() {

    char*  str  = "Welcome to AVOS!";
    char  attr  = 0x04;
    int   line  = 0;
	int    pos  = 32; 

	cls();
    
	print(str, attr, line, pos);

    return;
}

