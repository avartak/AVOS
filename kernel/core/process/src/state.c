#include <kernel/core/process/include/state.h>

struct IRQLock State_lock;

size_t State_CPUBlockSize() {
	return sizeof(struct State) + sizeof(struct CPU);
}
