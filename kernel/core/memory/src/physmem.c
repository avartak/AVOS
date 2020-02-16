#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/multiboot/include/multiboot.h>
#include <kernel/core/synch/include/irqlock.h>

struct IRQLock Page_operation_lock;

uint8_t* Page_maps[] = {
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x10000]  ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x20000]  ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x40000]  ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x80000]  ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x100000] ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x200000] ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x400000] ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x800000] ){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x1000000]){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x2000000]){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x4000000]){}
};

size_t Page_maps_size[] = {
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x10000  ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x20000  ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x40000  ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x80000  ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x100000 ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x200000 ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x400000 ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x800000 ,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x1000000,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x2000000,
	(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x4000000
};

struct Page_List* Page_lists[] = {
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL,
	PAGE_LIST_NULL
};

void Page_Release(void* pointer, uint8_t order) {

	IRQLock_Acquire(&Page_operation_lock);

	uintptr_t addr = (uintptr_t)pointer - KERNEL_HIGHER_HALF_OFFSET;
	uint8_t i = order;
	while (true) {
		PAGE_MAP_TOGGLEBIT(addr, i);
		if (i == PAGE_MAX_ORDER || PAGE_MAP_ISBITSET(addr, i)) break;
		i++;
	}	
	
	uint8_t j = order;
	while (j < i) {
		struct Page_List* buddy = (struct Page_List*)(PAGE_BUDDY(addr, j) + KERNEL_HIGHER_HALF_OFFSET);
		if (buddy->prev != PAGE_LIST_NULL) buddy->prev->next = buddy->next;
		if (buddy->next != PAGE_LIST_NULL) buddy->next->prev = buddy->prev;
		if (buddy->prev == PAGE_LIST_NULL) Page_lists[j] = buddy->next;
		buddy->prev = (struct Page_List*)0;
		buddy->next = (struct Page_List*)0;
		j++;
	}
	
	struct Page_List* pagelist_ptr = (struct Page_List*)( (addr & ~((1 << (12+i)) - 1)) + KERNEL_HIGHER_HALF_OFFSET );
	pagelist_ptr->prev = PAGE_LIST_NULL;
	pagelist_ptr->next = Page_lists[i];
	if (Page_lists[i] != PAGE_LIST_NULL) Page_lists[i]->prev = pagelist_ptr;
	Page_lists[i] = pagelist_ptr;

	IRQLock_Release(&Page_operation_lock);
}

void* Page_Acquire(uint8_t order) {

	IRQLock_Acquire(&Page_operation_lock);

	uint8_t i = order;
	for (; i <= PAGE_MAX_ORDER; i++) {
		if (Page_lists[i] != PAGE_LIST_NULL) break;
		else if (i == PAGE_MAX_ORDER) return PAGE_LIST_NULL;
	}
	
	void* page = Page_lists[i];
	uintptr_t addr = (uintptr_t)page - KERNEL_HIGHER_HALF_OFFSET;
	for (int8_t j = i; j >= order; j--) {
		PAGE_MAP_TOGGLEBIT(addr, j);
		if (j == i) {
			Page_lists[j] = Page_lists[j]->next;
			if (Page_lists[j] != PAGE_LIST_NULL) Page_lists[j]->prev = PAGE_LIST_NULL;
		}
		else {
			uintptr_t breakup_addr = addr + (1 << (12+j));
			Page_lists[j] = (struct Page_List*)(breakup_addr + KERNEL_HIGHER_HALF_OFFSET);
			Page_lists[j]->next = PAGE_LIST_NULL;
			Page_lists[j]->prev = PAGE_LIST_NULL;
		}
	}

	IRQLock_Release(&Page_operation_lock);
	return page;
}

void Page_MapMemoryChunk(uint64_t chunk_base, uint64_t chunk_size) {
	if (chunk_size == 0) return;
	if (chunk_base < KERNEL_MMAP_LOWMEM_END - KERNEL_HIGHER_HALF_OFFSET) return;
	if (chunk_base >= KERNEL_MMAP_VIRTUAL_END - KERNEL_HIGHER_HALF_OFFSET) return;
	
	uint32_t base = (uint32_t)chunk_base;
	uint32_t size = (uint32_t)(chunk_base + chunk_size > 0x100000000 ? 0x100000000 - chunk_base : chunk_size);
	
	uint8_t max_order = 0;
	for (uint8_t i = 0; i <= PAGE_MAX_ORDER; i++) {
		if (PAGE_SIZE_AT_ORDER(i) <= size) max_order = i;
	}
	
	uint32_t base_lowskim = base;
	uint32_t size_lowskim = (base % PAGE_SIZE_AT_ORDER(max_order) == 0 ? 0 : PAGE_SIZE_AT_ORDER(max_order) - (base % PAGE_SIZE_AT_ORDER(max_order)));
	
	uint32_t base_topskim = (base + size) & ~(PAGE_SIZE_AT_ORDER(max_order)-1);
	uint32_t size_topskim = base + size - base_topskim;
	
	uint32_t base_central = base_lowskim + size_lowskim;
	uint32_t size_central = base_topskim - base_central;
	
	Page_MapMemoryChunk(base_topskim, size_topskim);
	for (size_t i = 0; i < size_central/PAGE_SIZE_AT_ORDER(max_order); i++) {
		uintptr_t page_ptr = base_central + (size_central/PAGE_SIZE_AT_ORDER(max_order) - 1 - i) * PAGE_SIZE_AT_ORDER(max_order);
		struct Page_List* plist = (struct Page_List*)(page_ptr + KERNEL_HIGHER_HALF_OFFSET);
		
		plist->prev = PAGE_LIST_NULL;
		plist->next = Page_lists[max_order];
		if (Page_lists[max_order] != PAGE_LIST_NULL) Page_lists[max_order]->prev = plist;
		Page_lists[max_order] = plist;
		PAGE_MAP_SETBIT(page_ptr, max_order);
	}
	Page_MapMemoryChunk(base_lowskim, size_lowskim);
}

void Page_BuddyMaps_Initialize() {

	for (size_t i = 0; i <= PAGE_MAX_ORDER; i++) {
		for (size_t j = 0; j < Page_maps_size[i]; j++) {
			Page_maps[i][j] = 0;
		}
	}

	extern struct Multiboot_Info_Start* BootInfo_Ptr;
	
	struct Multiboot_Info_Tag* mbi_tag;
	uintptr_t mbi_addr = (uintptr_t)BootInfo_Ptr;
	struct Multiboot_RAMInfo_Entry* mmap = (struct Multiboot_RAMInfo_Entry*)0;
	size_t mmap_nentries = 0;
	
	mmap_nentries = 0;
	for (mbi_tag = (struct Multiboot_Info_Tag*)(mbi_addr + 8); mbi_tag->type != 0; mbi_tag = (struct Multiboot_Info_Tag*)((uint8_t*)mbi_tag + ((mbi_tag->size + 7) & ~7))) {
		if (mbi_tag->type == MULTIBOOT_TAG_TYPE_RAM_INFO_PAGE_ALIGNED) {
			struct Multiboot_Info_Memory_E820* mbi_tag_mmap = (struct Multiboot_Info_Memory_E820*)mbi_tag;
			mmap = (struct Multiboot_RAMInfo_Entry*)(mbi_tag_mmap->entries);
			mmap_nentries = (mbi_tag_mmap->entry_size == 0 ? 0 : (mbi_tag_mmap->size - 16)/(mbi_tag_mmap->entry_size));
		}
	}
	if (mmap == (struct Multiboot_RAMInfo_Entry*)0|| mmap_nentries == 0) return;

	for (size_t i = 0; i < mmap_nentries; i++) Page_MapMemoryChunk(mmap[mmap_nentries-1-i].address, mmap[mmap_nentries-1-i].size);

	IRQLock_Initialize(&Page_operation_lock);
}
