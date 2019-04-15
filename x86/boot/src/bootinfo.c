#include <x86/boot/include/bootinfo.h>

bool Boot_Info_Prepare() {

	struct Boot_Info_Entry* binfo = &Boot_Info;

	binfo[0].address = (uintptr_t)(&VBE_Table);
	uintptr_t next = VBE_StoreInfo(binfo[0].address);
	if (next == 0) return false;
	binfo[0].size = next - binfo[0].address;

	binfo[1].address = next;
	next = E820_StoreMap(next); 
	if (next == 0) return false;
	binfo[1].size = next - binfo[1].address;

    binfo[2].address = next;
    next = RAM_StoreMap(next);
    if (next == 0) return false;
    binfo[2].size = next - binfo[2].address;

	binfo[3].address = 0xFFFFFFFF;
	binfo[3].size    = 0xFFFFFFFF;

	return true;

}
