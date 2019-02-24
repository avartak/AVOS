#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include <kernel/include/machine.h>
#include <kernel/include/ioports.h>
#include <kernel/include/messaging.h>

struct Process;

struct Process_Node {
	struct Process_Node* next;
	struct Process* process;
};

struct Process_Attributes {
	uint32_t id;
	uint32_t flags;
	uint32_t owner;
	uint32_t group;
	struct Process* parent;
	struct Process_Node* children;
};

struct Process_Scheduling {
	uint32_t priority_current;
	uint32_t priority_best;

	uint32_t ticks_system_bill;
	uint32_t ticks_process_bill;
	uint32_t ticks_quantum_thiscycle;
	uint32_t ticks_consumed_thiscycle;

	uint32_t run_condition;
}

struct Process_Memory_PagingEntry {
	struct Process_Memory_PagingEntry* next;
	size_t    entry;
	uintptr_t address;
	uint32_t  attrib;
};

struct Process_Memory {
	uintptr_t start_code;
	uintptr_t start_data;
	uintptr_t start_readonly;
	uintptr_t start_bss;
	uintptr_t start_heap;

	uintptr_t end_code;
	uintptr_t end_data;
	uintptr_t end_readonly;
	uintptr_t end_bss;
	uintptr_t end_heap;

	uintptr_t top_stack;

	struct Process_Memory_PagingEntry* paging_directory_map;
};

struct Process_Message_Depot {
	struct Message msg;
	struct Process_Node* callers;
};

struct Process {
	struct Process_Attributes attrib;

	struct Process_State state;

	struct Process_Memory memory;
	struct IOPort_Node* ioports;

	struct Process_Scheduling sched;
	struct Process_Message_Depot postbox;

	struct File_Node* files;
}


#endif
