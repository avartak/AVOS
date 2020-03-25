#include <kernel/arch/processor/include/flags.h>

bool InterruptsEnabled() {

	return ( (ReadEFlags() & EFLAGS_IF) > 0 );
}
