#ifndef KERNEL_CORE_DISPATCHER_H
#define KERNEL_CORE_DISPATCHER_H

struct Process;

extern void Dispatch(struct Process* proc);
extern void Dispatcher_TerminateProcess(struct Process* proc, int exit_status);

#endif
