#include <kernel/arch/cpu/include/context.h>

size_t Context_GetStructSize() {

	return sizeof(struct Context);
}

void Context_SetProgramCounter(struct Context* context, uintptr_t ptr) {

	context->eip = ptr;
}
