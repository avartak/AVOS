#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/process.h>

struct SpinLock State_lock;

void State_Initialize(struct Process* proc) {

    struct State* state = STATE_FROM_PROC(proc);

    state->process = proc;
    state->preemption_vetos = 0;
    state->interrupt_priority = 0;
}
