#include <kernel/core/process/include/state.h>

struct SpinLock State_lock;

size_t State_CPUBlockSize() {
	return sizeof(struct State) + sizeof(struct CPU);
}
