#ifndef KERNEL_CORE_SCHEDULER_H
#define KERNEL_CORE_SCHEDULER_H

#include <stdint.h>

#include <kernel/core/process/include/process.h>

extern struct Process  Scheduler_processes[];

extern void            Scheduler_Initialize();
extern void            Scheduler_RunProcess(struct Process* proc);
extern void            Scheduler_Return();
extern struct Process* Scheduler_Book();
extern void            Scheduler_ChangeParent(struct Process* old_parent, struct Process* new_parent);
extern void            Scheduler_Wakeup(void* alarm);
extern bool            Scheduler_ProcessHasKids(struct Process* proc);
extern struct Process* Scheduler_GetProcessKid(struct Process* proc, enum Process_LifeCycle cycle);

extern void            Schedule()__attribute__ ((noreturn));

#endif
