/*

This is where the action really starts!

The first thing the kernel does is to perform low-level initialization of the system
This low level initialization will be done by the Kinit() function 
Since this initialization is architecture specific, the code resides in the x86 folder

As of now, the kernel does not do anything beyond Kinit() and simply returns to Kstart which in turn puts the system into a sleep loop

*/



#include <x86/kernel/include/kinit.h>

void Kmain() {

	Kinit();
	
    return;
}

