#include <x86/boot/include/info.h>

void BootInfo_Store(uintptr_t info_ptr, uintptr_t tables_addr) {

	struct Info_Entry* binfo = (struct Info_Entry*)info_ptr ;
	uintptr_t next = tables_addr;

	binfo[0].address = next;
	next = VBE_StoreInfo(next);
	binfo[0].size = next - binfo[0].address;

	binfo[1].address = next;
	next = E820_StoreInfo(next); 
	binfo[1].size = next - binfo[1].address;

    binfo[2].address = next;
    next = RAM_StoreInfo(next);
    binfo[2].size = next - binfo[2].address;

}
