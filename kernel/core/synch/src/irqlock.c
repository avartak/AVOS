#include <kernel/core/synch/include/irqlock.h>
#include <kernel/core/process/include/state.h>
#include <kernel/arch/console/include/console.h>
#include <kernel/arch/i386/include/functions.h>

void IRQLock_Initialize(struct IRQLock* lock, const char* name) {
	SpinLock_Initialize(&(lock->lock), name);	
	lock->interrupt_priority = 0;
}

void IRQLock_Acquire(struct IRQLock* lock) {
	State_GetCurrent()->interrupt_priority = X86_DisableInterrupts();
	SpinLock_Acquire(&(lock->lock));
}

void IRQLock_Release(struct IRQLock* lock) {
	SpinLock_Release(&(lock->lock));
	X86_RestoreInterrupts(State_GetCurrent()->interrupt_priority);
}

