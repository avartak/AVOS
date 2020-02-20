#include <kernel/core/synch/include/irqlock.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/arch/include/arch.h>

void IRQLock_Initialize(struct IRQLock* lock) {
	SpinLock_Initialize(&(lock->lock));	
	lock->previous_interrupt_priority = 0;
}

void IRQLock_Acquire(struct IRQLock* lock) {
	lock->previous_interrupt_priority = (InterruptsEnabled() > 0 ? 0 : 0xFF);
	DisableInterrupts();
	SpinLock_Acquire(&(lock->lock));
	STATE_CURRENT->interrupt_priority = 0xFF; 
}

void IRQLock_Release(struct IRQLock* lock) {
	STATE_CURRENT->interrupt_priority = lock->previous_interrupt_priority; 
	SpinLock_Release(&(lock->lock));
	if (STATE_CURRENT->interrupt_priority != 0xFF) EnableInterrupts();
}

