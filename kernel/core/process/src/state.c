#include <kernel/core/process/include/state.h>
#include <kernel/arch/initial/include/kthread.h>

struct State* State_GetCurrent() {
	
	struct State* kstate = (struct State*)KernelThread_GetStackBase();
	kstate--;
	return kstate;
}
