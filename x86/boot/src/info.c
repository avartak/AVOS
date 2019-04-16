#include <x86/boot/include/info.h>

extern uint8_t Boot_Tables;

bool BootInfo_Store() {

	struct Info_Entry* binfo = &BootInfo_Table;

	uintptr_t next = (uintptr_t)(&Boot_Tables);

	binfo[0].address = next;
	next = VBE_StoreInfo(next);
	if (next == 0) return false;
	binfo[0].size = next - binfo[0].address;

	binfo[1].address = next;
	next = E820_StoreInfo(next); 
	if (next == 0) return false;
	binfo[1].size = next - binfo[1].address;

    binfo[2].address = next;
    next = RAM_StoreInfo(next);
    if (next == 0) return false;
    binfo[2].size = next - binfo[2].address;

	return true;

}
