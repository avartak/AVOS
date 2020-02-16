#include <kernel/arch/i386/include/flags.h>

bool InterruptsEnabled() {

	return ( (X86_ReadEFlags() & X86_EFLAGS_IF) > 0 );
}
