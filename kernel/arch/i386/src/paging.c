#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/arch/include/arch.h>
#include <kernel/clib/include/string.h>
#include <kernel/arch/i386/include/paging.h>

bool Paging_IsPageMapped(uintptr_t* page_dir, uintptr_t virt_addr) {

    uintptr_t* page_table;
    uintptr_t* page_dir_entry = &page_dir[X86_PAGING_PGDIR_IDX(virt_addr)];

    if ( !(*page_dir_entry & X86_PAGING_PDE_PRESENT) ) return false;
	page_table = (uintptr_t*)( (*page_dir_entry & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
	if (page_table[X86_PAGING_PGTAB_IDX(virt_addr)] == 0) return false;

	return true;
}

bool Paging_MapPage(uintptr_t* page_dir, uintptr_t virt_addr, uintptr_t phys_addr, bool create) {

	uintptr_t* page_table;
	uintptr_t* page_dir_entry = &page_dir[X86_PAGING_PGDIR_IDX(virt_addr)];

	if ( !(*page_dir_entry & X86_PAGING_PDE_PRESENT) ) {

		if (!create) return false;

		page_table = Page_Acquire(0);
		if (page_table == (uintptr_t*)PAGE_LIST_NULL) return false;
		memset(page_table, 0, X86_PAGING_PAGESIZE);
		*page_dir_entry = ((uintptr_t)page_table - KERNEL_HIGHER_HALF_OFFSET) | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_USER;
	}

	else {
		page_table = (uintptr_t*)( (*page_dir_entry & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
		if (page_table[X86_PAGING_PGTAB_IDX(virt_addr)] != 0) return false;
	}

	page_table[X86_PAGING_PGTAB_IDX(virt_addr)] = phys_addr | X86_PAGING_PTE_PRESENT | X86_PAGING_PTE_READWRITE | X86_PAGING_PTE_USER;
	return true;
}

bool Paging_UnmapPage(uintptr_t* page_dir, uintptr_t virt_addr) {

    uintptr_t* page_dir_entry = &page_dir[X86_PAGING_PGDIR_IDX(virt_addr)];

    if ( !(*page_dir_entry & X86_PAGING_PDE_PRESENT) ) return false;

	uintptr_t* page_table = (uintptr_t*)( (*page_dir_entry & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
	uintptr_t* page_table_entry = &page_table[X86_PAGING_PGTAB_IDX(virt_addr)];
	if ( !(*page_table_entry & X86_PAGING_PTE_PRESENT) ) return false;

	uint8_t* page = (uint8_t*)( (*page_table_entry & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
	Page_Release(page, 0);
	*page_table_entry = 0;
    return true;
}

size_t Paging_MapPages(uintptr_t* page_dir, void* virt_addr, size_t size) {
	
	uintptr_t va = (uintptr_t)virt_addr & ~(X86_PAGING_PAGESIZE-1);
	uintptr_t va_end = (uintptr_t)(virt_addr + size - 1) & ~(X86_PAGING_PAGESIZE-1);
	size_t sz = 0;

	while (true) {

		void* page_ptr = Page_Acquire(0);
		if (page_ptr == PAGE_LIST_NULL) break;
		uintptr_t pa = (uintptr_t)page_ptr - KERNEL_HIGHER_HALF_OFFSET;

		if (!Paging_MapPage(page_dir, va, pa, true)) {
			Page_Release(page_ptr, 0);
			break;
		}

		sz += X86_PAGING_PAGESIZE;
		if (va == va_end) break;

		va += X86_PAGING_PAGESIZE;
		pa += X86_PAGING_PAGESIZE;
	}

	return sz;
}

bool Paging_UnmapPages(uintptr_t* page_dir, void* virt_addr, size_t size) {
    
    uintptr_t va = (uintptr_t)(virt_addr + X86_PAGING_PAGESIZE - 1) & ~(X86_PAGING_PAGESIZE-1);
    uintptr_t va_end = (uintptr_t)(virt_addr + size - 1) & ~(X86_PAGING_PAGESIZE-1);

	bool unmapping_failed = false;

    while (true) {
		if (va >= KERNEL_MMAP_VIRTUAL_START) {
			unmapping_failed = true;
			break;
		}
        if (va > va_end) break;
        if (!Paging_UnmapPage(page_dir, va)) unmapping_failed = true;
        va += X86_PAGING_PAGESIZE;
    }

    return unmapping_failed;
}

bool Paging_MakePageDirectory(uintptr_t* page_dir) {

    uintptr_t* pgd = Page_Acquire(0);
    if (pgd == (uintptr_t*)PAGE_LIST_NULL) return false;

    page_dir = pgd;
    memset(page_dir, 0, X86_PAGING_PAGESIZE);

    for (size_t i = KERNEL_MMAP_VIRTUAL_START; i > 0; i+=X86_PAGING_EXTPAGESIZE) page_dir[X86_PAGING_PGDIR_IDX(i)] = Kernel_pagedirectory[X86_PAGING_PGDIR_IDX(i)];

    return true;
}

void Paging_UnmakePageDirectory(uintptr_t* page_dir) {

	Paging_UnmapPages(page_dir, (void*)0, KERNEL_MMAP_VIRTUAL_START);
	for (size_t i = 0; i < KERNEL_MMAP_VIRTUAL_START; i+=X86_PAGING_EXTPAGESIZE) {
		uintptr_t page_table = (page_dir[X86_PAGING_PGDIR_IDX(i)] & ~(X86_PAGING_PAGESIZE-1) ) + KERNEL_HIGHER_HALF_OFFSET;
		Page_Release((void*)page_table, 0);
		page_dir[X86_PAGING_PGDIR_IDX(i)] = 0;
	}

	Page_Release(page_dir, 0);
}

/*
bool Paging_SetPageFlags(uintptr_t* page_dir, void* virt_addr, uint16_t flags) {

	if (!Paging_IsPageMapped(page_dir, (uintptr_t)virt_addr)) return false;

	uintptr_t* page_table = (uintptr_t*)( (page_dir[X86_PAGING_PGDIR_IDX(virt_addr)] & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
	page_table[X86_PAGING_PGTAB_IDX(virt_addr)] |= flags;
	return true;
}

bool Paging_UnsetPageFlags(uintptr_t* page_dir, void* virt_addr, uint16_t flags) {

    if (!Paging_IsPageMapped(page_dir, (uintptr_t)virt_addr)) return false;

    uintptr_t* page_table = (uintptr_t*)( (page_dir[X86_PAGING_PGDIR_IDX(virt_addr)] & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
    page_table[X86_PAGING_PGTAB_IDX(virt_addr)] &= ~flags;
	return true;
}
*/

bool Paging_ClonePageDirectory(uintptr_t* page_dir, uintptr_t* clone) {

	if (!Paging_MakePageDirectory(page_dir)) return false;

	bool cloning_failed = false;
	for (size_t i = 0; i < X86_PAGING_PGDIR_NENTRIES; i++) {
		if (page_dir[i] == 0) continue;

		void* page_table = Page_Acquire(0);
		if (page_table == PAGE_LIST_NULL) {
			cloning_failed = true;
			break;
		}
		uintptr_t page_table_flags = page_dir[i] & (X86_PAGING_PAGESIZE-1);
		clone[i] = ((uintptr_t)page_table - KERNEL_HIGHER_HALF_OFFSET) + page_table_flags;

		uintptr_t* ptab = (uintptr_t*)( (page_dir[i] & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
		for (size_t j = 0; j < X86_PAGING_PGTAB_NENTRIES; j++) {
			if (ptab[j] == 0) continue;

			void* page = Page_Acquire(0);
			if (page == PAGE_LIST_NULL) {
				cloning_failed = true;
				break;
			}
			uintptr_t page_flags = ptab[j] & (X86_PAGING_PAGESIZE-1);
			((uintptr_t*)page_table)[j] = ((uintptr_t)page - KERNEL_HIGHER_HALF_OFFSET) + page_flags;
			
			memmove(page, (void*)((ptab[j] & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET), X86_PAGING_PAGESIZE);
		}
		if (cloning_failed) break;
	}

	if (cloning_failed) {
		Paging_UnmakePageDirectory(clone);
		return false;
	}

	return true;
}

uintptr_t Paging_GetHigherHalfAddress(uintptr_t* page_dir, uintptr_t user_addr) {

	if (user_addr > KERNEL_MMAP_VIRTUAL_START)
	if (!Paging_IsPageMapped(page_dir, user_addr)) return 0;

    uintptr_t* page_table = (uintptr_t*)( (page_dir[X86_PAGING_PGDIR_IDX(user_addr)] & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET );
	if ((page_table[X86_PAGING_PGTAB_IDX(user_addr)] & X86_PAGING_PTE_USER) == 0) return 0;

    return (page_table[X86_PAGING_PGTAB_IDX(user_addr)] & ~(X86_PAGING_PAGESIZE-1)) + KERNEL_HIGHER_HALF_OFFSET + (user_addr & (X86_PAGING_PAGESIZE-1));
}

bool Paging_Copy(uintptr_t* dest_page_dir, uintptr_t dest_addr, uintptr_t src_addr, size_t nbytes) {

	bool success = true;

	while (nbytes > 0) {
		uintptr_t dest_hhalf_addr = Paging_GetHigherHalfAddress(dest_page_dir, dest_addr);
		if (dest_hhalf_addr == 0) {
			success = false;
			break;
		}

		size_t bytes_in_this_page = X86_PAGING_PAGESIZE - (dest_addr & (X86_PAGING_PAGESIZE-1));
		if (nbytes < bytes_in_this_page) bytes_in_this_page = nbytes;

		memmove((void*)dest_hhalf_addr, (void*)src_addr, bytes_in_this_page);

		dest_addr += bytes_in_this_page;
		nbytes -= bytes_in_this_page;
	}

	return success;
}

uint16_t Paging_StandardUserPageFlags() {

	return X86_PAGING_PTE_PRESENT | X86_PAGING_PTE_READWRITE | X86_PAGING_PTE_USER;
}
