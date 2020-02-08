#ifndef KERNEL_CORE_SCHEDULER_H
#define KERNEL_CORE_SCHEDULER_H

#include <stdint.h>

#include <kernel/core/process/include/process.h>
#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/synch/include/irqlock.h>
#include <kernel/core/synch/include/sleeplock.h>

extern struct Process  Scheduler_processes[];

extern void            Scheduler_Initialize();
extern void            Scheduler_RunProcess(struct Process* proc);
extern void            Scheduler_Return();
extern bool            Scheduler_ProcessShouldYield(struct Process* proc);
extern struct Process* Scheduler_Book();
extern void            Scheduler_ChangeParent(struct Process* old_parent, struct Process* new_parent);
extern void            Scheduler_Wakeup(struct Process* alarm);
extern void            Scheduler_WakeupFromSleepLock(struct SleepLock* lock);
extern bool            Scheduler_ProcessHasKids(struct Process* proc);
extern struct Process* Scheduler_GetProcessKid(struct Process* proc, enum Process_LifeCycle cycle);
extern void            Scheduler_TerminateProcess(struct Process* proc);
extern void            Scheduler_KillProcess(uint32_t process_id);

extern void            Schedule()__attribute__ ((noreturn));

#endif
