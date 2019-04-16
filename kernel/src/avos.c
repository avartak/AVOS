/*

This is where the action really starts!

The kernel performs system initialization and goes into a sleep loop.
The sleep loop is basically a halt instruction followed by a jump to the halt instruction. Why not just a halt ?
Lets assume we got to the point of the sleep loop. At that point the system is in an idle state
The only thing that will wake it up is an interrupt -- for example, lets say you press a key
Now, the interrupt will do what it needs to do and then return to the next instruction after the point where it injected itself
If the system is sitting at its very last instruction, there is no next instruction, then it will go crazy 
The sleep loop makes sure that does not happen.

If the kernel ever gets out of the sleep loop, we put the system into deep sleep
All interrupts are disabled and the system returns to the calling Kstart function which halts the system 

*/

#include <kernel/include/memory.h>
#include <kernel/include/interrupts.h>
#include <kernel/include/timer.h>
#include <kernel/include/drivers.h>
#include <kernel/include/machine.h>

void AVOS() {

	Memory_Initialize();
	
	Interrupt_Initialize();
	
	Drivers_Load();
	
	Timer_Initialize();

	Interrupt_EnableAll();

	while (1) {
		System_Halt();
	} 
	
}

