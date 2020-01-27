#include <kernel/core/memory/include/physmem.h>

uint8_t* Page_maps[] = {
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x10000])  {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x20000])  {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x40000])  {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x80000])  {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x100000]) {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x200000]) {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x400000]) {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x800000]) {},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x1000000]){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x2000000]){},
	(uint8_t [(KERNEL_MMAP_VIRTUAL_END-KERNEL_MMAP_VIRTUAL_START)/0x4000000]){}
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

	uintptr_t addr = (uintptr_t)pointer - KERNEL_HIGHER_HALF_OFFSET;
	uint8_t i = order;
	while (true) {
		PAGE_MAP_TOGGLEBIT(addr, i);
		if (i == PAGE_MAX_ORDER || PAGE_MAP_ISBITSET(addr, i)) break;
		i++;
	}	

	uint8_t j = order;
	while (j <= i) {
		struct Page_List* buddy = (struct Page_List*)PAGE_BUDDY(addr, order);
		buddy->prev->next = buddy->next;
		buddy->next->prev = buddy->prev;
		buddy->prev = (struct Page_List*)0;
		buddy->next = (struct Page_List*)0;
	}

	struct Page_List* pagelist_ptr = (struct Page_List*)(addr & ~(~((1 << (12+i)) - 1)));
	pagelist_ptr->prev = PAGE_LIST_NULL;
	pagelist_ptr->next = Page_lists[i];
	if (Page_lists[i] != PAGE_LIST_NULL) Page_lists[i]->prev = pagelist_ptr;
	Page_lists[i] = pagelist_ptr;
}

void* Page_Acquire(uint8_t order) {

	uint8_t i = order;
	for (; i <= PAGE_MAX_ORDER; i++) {
		if (Page_lists[i] != PAGE_LIST_NULL) break;
		else if (i == PAGE_MAX_ORDER) return PAGE_LIST_NULL;
	}

	void* page = Page_lists[i];
	uintptr_t breakup_addr = (uintptr_t)page;
	for (uint8_t j = i; j >= order; j--) {
		PAGE_MAP_TOGGLEBIT(breakup_addr, j);
		if (j == i) {
			Page_lists[j] = Page_lists[j]->next;
			if (Page_lists[j] != PAGE_LIST_NULL) Page_lists[j]->prev = PAGE_LIST_NULL;
		}
		else {
			breakup_addr += 1 << (12+j);
			Page_lists[j] = (struct Page_List*)(breakup_addr);
			Page_lists[j]->next = PAGE_LIST_NULL;
			Page_lists[j]->prev = PAGE_LIST_NULL;
		}
	}
	return (void*)((uintptr_t)page + KERNEL_HIGHER_HALF_OFFSET);
}

