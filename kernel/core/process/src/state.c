#include <kernel/core/process/include/state.h>
#include <kernel/arch/i386/include/functions.h>

struct State* State_GetCurrent() {
	return (struct State*)(X86_GetStackBase() - sizeof(struct State));
}

struct CPU* CPU_GetCurrent() {
    return (struct CPU*)(X86_GetStackBase() - sizeof(struct State) - sizeof(struct CPU));
}
