/*

This is where the action really starts!

The first thing the kernel does is to perform low-level initialization of the system
Some of this may have been done already by the boot loader
But we are not going to assume anything since we would rather stay as independent of the boot loader as possible
The only things to assume are that 
- We are in protected mode already
- The kernel is mapped to memory 0xC0100000
- Interrupts have been disabled

This low level initialization will be done by the Kinit() function 
Since this initialization is architecture specific, the code resides in the x86 folder

As of now, the kernel does not do anything beyond Kinit() and simply returns to Kstart which in turn puts the system into a sleep loop

*/



#include <x86/kernel/include/kinit.h>

void Kmain() {

	Kinit();
	
    return;
}

