#include <kernel/core/synch/include/irqlock.h>
#include <kernel/core/process/include/state.h>
#include <kernel/arch/console/include/console.h>
#include <kernel/arch/i386/include/functions.h>
#include <kernel/arch/i386/include/flags.h>

void IRQLock_Initialize(struct IRQLock* lock, const char* name) {
	SpinLock_Initialize(&(lock->lock), name);	
	lock->previous_interrupt_priority = 0;
}

void IRQLock_Acquire(struct IRQLock* lock) {
	lock->previous_interrupt_priority = ((X86_ReadEFlags() & X86_EFLAGS_IF) > 0 ? 0 : 0xFF);
	X86_DisableInterrupts();
	SpinLock_Acquire(&(lock->lock));
	STATE_CURRENT->interrupt_priority = 0xFF; 
}

void IRQLock_Release(struct IRQLock* lock) {
	STATE_CURRENT->interrupt_priority = lock->previous_interrupt_priority; 
	SpinLock_Release(&(lock->lock));
	if (STATE_CURRENT->interrupt_priority != 0xFF) X86_EnableInterrupts();
}

