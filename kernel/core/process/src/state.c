#include <kernel/core/process/include/state.h>
#include <kernel/arch/i386/include/functions.h>

struct State* State_GetCurrent() {
	
	struct State* kstate = (struct State*)X86_GetStackBase();
	kstate--;
	return kstate;
}
