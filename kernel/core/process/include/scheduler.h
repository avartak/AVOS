#ifndef KERNEL_CORE_SCHEDULER_H
#define KERNEL_CORE_SCHEDULER_H

#include <kernel/core/process/include/process.h>

extern struct Process Scheduler_processes[];

extern void Scheduler_Initialize();
extern void Scheduler_RunProcess(struct Process* proc);
extern void Scheduler_Return();
extern void Schedule()__attribute__ ((noreturn));

#endif
