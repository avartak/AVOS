#ifndef KERNEL_CORE_SIGNAL_H
#define KERNEL_CORE_SIGNAL_H

#include <stdint.h>
#include <stdbool.h>

#include <kernel/core/setup/include/setup.h>

struct Signal {
	int32_t   id;             // I would have preferred a uint32_t but POSIX says int, and (at least for now) it does not matter so int it is  
	sigset_t  mask;
	uintptr_t handler_1arg;
	uintptr_t handler_3arg3;
}__attribute__((packed));

struct Signal_Info {
	int32_t   id;
	pid_t     process;
	uint32_t  user;
	int32_t   code;
	int32_t   error;
	int32_t   status;
	long      band;           // Will worry about this later
	void*     address;
}__attribute__((packed));

struct Process;

extern bool Signal_Transmit(pid_t proc_id);

#endif
