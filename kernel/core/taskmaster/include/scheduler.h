#ifndef KERNEL_CORE_SCHEDULER_H
#define KERNEL_CORE_SCHEDULER_H

#include <stdint.h>

#include <kernel/core/arch/include/arch.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/synch/include/irqlock.h>
#include <kernel/core/synch/include/sleeplock.h>

#define SCHEDULER_RETURN       Context_Switch(&(STATE_CURRENT->process->context), STATE_CURRENT->kernel_task->scheduler)
#define SCHEDULER_SWITCH(proc) Context_Switch(&(STATE_CURRENT->kernel_task->scheduler), (proc)->context)

extern struct Process  Scheduler_processes[];

extern void            Scheduler_Initialize();
extern struct Process* Scheduler_Book();
extern void            Scheduler_RaiseAlarm(void* alarm);
extern bool            Scheduler_Preempt(struct Process* proc);

extern void            Schedule()__attribute__ ((noreturn));

#endif
