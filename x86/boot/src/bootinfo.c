#include <x86/boot/include/bootinfo.h>

bool Boot_Info_Store() {

	struct Boot_Info_Entry* binfo = &Boot_Info;

	binfo[0].address = (uintptr_t)(&VBE_Table);
	uintptr_t next = VBE_StoreInfo(binfo[0].address);
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
