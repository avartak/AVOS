#include <kernel/include/timer.h>
#include <kernel/include/memory.h>
#include <kernel/include/interrupts.h>
#include <kernel/x86/kernel/include/welcome.h>

clock_t timerticks = 0;

uint32_t Timer_Ticks() {
	return timerticks;
}

uint32_t Timer_HandleInterrupt() {
	timerticks++;
	PrintNum(timerticks/50, 24, 0);
	return 2;
}

void Timer_Initialize() {
	timerticks = 0;
	struct Interrupt_Handler* handler = (struct Interrupt_Handler*)(Memory_Virtual_Allocate(sizeof(struct Interrupt_Handler)));	
	handler->next     = MEMORY_NULL_PTR;
	handler->handler  = &Timer_HandleInterrupt;
	handler->id       = 0;
	handler->process  = 0;	

	uint8_t retval = Interrupt_AddHandler(handler, 0x20);
	if (retval == 0 || retval == 1) Memory_Virtual_Free((uintptr_t)handler);
}
