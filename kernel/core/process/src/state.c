#include <kernel/core/process/include/state.h>
#include <kernel/arch/i386/include/functions.h>

struct State* State_GetCurrent() {
	return (struct State*)(X86_GetStackBase() - sizeof(struct State));
}

struct CPU* State_GetCPU() {
    return State_GetCurrent()->cpu;
}
