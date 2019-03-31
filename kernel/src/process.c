#include <kernel/include/process.h>
#include <kernel/include/memory.h>

struct Process*      Process_current  = MEMORY_NULL_PTR;
struct Process*      Process_next     = MEMORY_NULL_PTR;
struct Process*      Process_previous = MEMORY_NULL_PTR;
struct Process*      Process_billable = MEMORY_NULL_PTR;
uintptr_t*           Process_map      = MEMORY_NULL_PTR;

struct Process_Node* Process_ready_queue_front[PROCESS_SCHED_QUEUES];
struct Process_Node* Process_ready_queue_back [PROCESS_SCHED_QUEUES];

void Process_Initialize() {
	Process_map = (uintptr_t*)(Memory_Virtual_Allocate(0x1000));
}

uintptr_t Process_GetKernelSwitchStackLocation(struct Process* proc) {
	return (uintptr_t)(&(proc->attrib));
}

struct Process* Process_FromID(size_t id) {
	if (id >= PROCESS_MAX_NUMBER) return MEMORY_NULL_PTR;
	else return (struct Process*)Process_map[id];
}

bool Process_Enqueue(struct Process* proc) {
	bool isInterruptEnabled = Interrupt_IsFlagSet();
	if (isInterruptEnabled) Interrupt_DisableAll();

	if (proc == MEMORY_NULL_PTR) {
		if (isInterruptEnabled) Interrupt_EnableAll();
		return false;
	}

	struct Process_Node* node = (struct Process_Node*)(Memory_Virtual_Allocate(sizeof(struct Process_Node)));
	if (node == MEMORY_NULL_PTR) {
		if (isInterruptEnabled) Interrupt_EnableAll();
		return false;	
	}

	int8_t sched_retval = Process_Schedule(proc);
	if (sched_retval == 0) {
		if (isInterruptEnabled) Interrupt_EnableAll();
		return false;
	}

	uint8_t q = (proc->sched).priority_current;
	if (q >= PROCESS_SCHED_QUEUES) {
		if (isInterruptEnabled) Interrupt_EnableAll();
		return false;
	}

	node->process = proc;
	node->next = MEMORY_NULL_PTR;

	if (Process_ready_queue_front[q] == MEMORY_NULL_PTR) {     
		Process_ready_queue_front[q] = Process_ready_queue_back[q] = node;
	} 
	else if (sched_retval > 0) {
		node->next = Process_ready_queue_front[q]; 
		Process_ready_queue_front[q] = node;              
	} 
	else {
		Process_ready_queue_back[q]->next = node;        
		Process_ready_queue_back[q] = node;              
	}
	Process_SelectNextToRun();

	if (isInterruptEnabled) Interrupt_EnableAll();
	return true;
}

bool Process_Dequeue(struct Process* proc) {
	bool isInterruptEnabled = Interrupt_IsFlagSet();
	if (isInterruptEnabled) Interrupt_DisableAll();

	if (proc == MEMORY_NULL_PTR) {
		if (isInterruptEnabled) Interrupt_EnableAll();
		return false;
	}

	uint8_t q = (proc->sched).priority_current;
	if (q >= PROCESS_SCHED_QUEUES) {
		if (isInterruptEnabled) Interrupt_EnableAll();
		return false;
	}

	struct Process_Node* node = MEMORY_NULL_PTR;
	if (Process_ready_queue_front[q]->process == proc) {
		node = Process_ready_queue_front[q];
		Process_ready_queue_front[q] = node->next;
	}

	while (Process_ready_queue_front[q]->next != MEMORY_NULL_PTR) {
		if (Process_ready_queue_front[q]->next->process == proc) {
			if (Process_ready_queue_front[q]->next == Process_ready_queue_back[q]) {
				node = Process_ready_queue_back[q];
				Process_ready_queue_front[q]->next = MEMORY_NULL_PTR;
			}
			else {
				node = Process_ready_queue_front[q]->next;
				Process_ready_queue_front[q]->next = Process_ready_queue_front[q]->next->next;
			}
		}
	}

	if (node != MEMORY_NULL_PTR && (proc == Process_current || proc == Process_next)) Process_SelectNextToRun();

	Memory_Virtual_Free((uintptr_t)node);

	if (isInterruptEnabled) Interrupt_EnableAll();
	return  true;	
}

int8_t Process_Schedule(struct Process* proc) {
	if (proc == MEMORY_NULL_PTR) return 0;

	bool time_left = ((proc->sched).ticks_consumed >= (proc->sched).ticks_quantum); 
	int8_t penalty = 0;
	static struct Process *prev_tover_prov = MEMORY_NULL_PTR;

	if (!time_left) {
		(proc->sched).ticks_consumed = 0;
		if (prev_tover_prov == proc) penalty++;
		else penalty--;
		prev_tover_prov = proc;
	}

	if (penalty != 0 && ((proc->attrib).flags & PROCESS_KERNEL) == 0) {
		(proc->sched).priority_current += penalty;
		if ((proc->sched).priority_current < (proc->sched).priority_best) (proc->sched).priority_current = (proc->sched).priority_best;
		if ((proc->sched).priority_current >= PROCESS_SCHED_QUEUES)       (proc->sched).priority_current = PROCESS_SCHED_QUEUES-1;
	}

	if (time_left) return 1;
	else return -1;
}

void Process_SelectNextToRun() {
	for (uint8_t q = 0; q < PROCESS_SCHED_QUEUES; q++) {
		if (Process_ready_queue_front[q] != MEMORY_NULL_PTR) {
			Process_next = Process_ready_queue_front[q]->process;
			if (((Process_next->attrib).flags & PROCESS_BILLABLE) != 0) Process_billable = Process_next;
			return;
		}
	}
}

