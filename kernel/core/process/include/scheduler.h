#ifndef KERNEL_CORE_SCHEDULER_H
#define KERNEL_CORE_SCHEDULER_H

#include <stdint.h>

#include <kernel/core/process/include/process.h>
#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/synch/include/irqlock.h>
#include <kernel/core/synch/include/sleeplock.h>

#define SCHEDULER_RETURN       Context_Switch(&(STATE_CURRENT->process->context), STATE_CURRENT->cpu->scheduler)
#define SCHEDULER_SWITCH(proc) Context_Switch(&(STATE_CURRENT->cpu->scheduler)  , (proc)->context)

extern struct Process  Scheduler_processes[];

extern void            Scheduler_Initialize();
extern void            Scheduler_HandleInterruptReturn();
extern struct Process* Scheduler_Book();
extern void            Scheduler_ChangeParent(struct Process* old_parent, struct Process* new_parent);
extern void            Scheduler_Wakeup(void* alarm);
extern struct Process* Scheduler_GetProcessKid(struct Process* proc, enum Process_LifeCycle cycle);
extern void            Scheduler_TerminateProcess(struct Process* proc);
extern void            Scheduler_LifeCycleChangeSignal(uint32_t process_id, enum Process_LifeCycle requested_cycle);

extern void            Schedule()__attribute__ ((noreturn));

#endif
