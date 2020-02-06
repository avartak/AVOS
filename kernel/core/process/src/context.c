#include <kernel/core/process/include/context.h>

void Context_SetProgramCounter(struct Context* context, uintptr_t ptr) {
	context->eip = ptr;
}
