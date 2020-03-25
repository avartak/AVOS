#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/taskmaster/include/process.h>
#include <kernel/core/arch/include/arch.h>
#include <kernel/clib/include/string.h>
#include <kernel/arch/tasking/include/context.h>
#include <kernel/arch/paging/include/paging.h>

#define PAGESIZE                PAGING_PAGESIZE
#define PAGEADDRESS(x)          ((uintptr_t)(x) & ~(PAGESIZE-1))
#define PAGEOFFSET(x)           ((uintptr_t)(x) &  (PAGESIZE-1))
#define PAGETABIDX(x)           PAGING_PGTAB_IDX(x) 
#define PAGEDIRIDX(x)           PAGING_PGDIR_IDX(x) 

bool Paging_IsPageMapped(struct Process* proc, uintptr_t virt_addr) {

    uintptr_t* page_table;
    uintptr_t  page_dir_entry = proc->task_context->cr3[PAGEDIRIDX(virt_addr)];

    if ( !(page_dir_entry & PAGING_PDE_PRESENT) ) return false;
    page_table = (uintptr_t*)KERNADDR(PAGEADDRESS(page_dir_entry));
    if (page_table[PAGETABIDX(virt_addr)] == 0) return false;

    return true;
}


bool Paging_MapPage(struct Process* proc, uintptr_t virt_addr, uintptr_t phys_addr, bool create) {

	uintptr_t* page_table;
	uintptr_t* page_dir_entry = &proc->task_context->cr3[PAGEDIRIDX(virt_addr)];

	if ( !(*page_dir_entry & PAGING_PDE_PRESENT) ) {

		if (!create) return false;

		page_table = Page_Acquire(0);
		if (page_table == (uintptr_t*)PAGE_LIST_NULL) return false;
		memset(page_table, 0, PAGESIZE);
		*page_dir_entry = PHYSADDR(page_table) | PAGING_PDE_PRESENT | PAGING_PDE_READWRITE | PAGING_PDE_USER;
	}

	else {
		page_table = (uintptr_t*)KERNADDR(PAGEADDRESS(*page_dir_entry));
		if (page_table[PAGETABIDX(virt_addr)] != 0) return false;
	}

	page_table[PAGETABIDX(virt_addr)] = phys_addr | PAGING_PTE_PRESENT | PAGING_PTE_READWRITE | PAGING_PTE_USER;
	return true;
}

bool Paging_UnmapPage(struct Process* proc, uintptr_t virt_addr) {

    uintptr_t* page_dir_entry = &proc->task_context->cr3[PAGEDIRIDX(virt_addr)];

    if ( !(*page_dir_entry & PAGING_PDE_PRESENT) ) return false;

	uintptr_t* page_table = (uintptr_t*)KERNADDR(PAGEADDRESS(*page_dir_entry));
	uintptr_t* page_table_entry = &page_table[PAGETABIDX(virt_addr)];
	if ( !(*page_table_entry & PAGING_PTE_PRESENT) ) return false;

	uint8_t* page = (uint8_t*)KERNADDR(PAGEADDRESS(*page_table_entry));
	Page_Release(page, 0);
	*page_table_entry = 0;
    return true;
}

size_t Paging_MapPages(struct Process* proc, void* virt_addr, size_t size) {

	if (size == 0) return 0;
	
	uintptr_t va = PAGEADDRESS(virt_addr);
	uintptr_t va_end = PAGEADDRESS(virt_addr + size - 1);
	size_t sz = 0;

	while (true) {

		void* page_ptr = Page_Acquire(0);
		if (page_ptr == PAGE_LIST_NULL) break;
		uintptr_t pa = PHYSADDR(page_ptr);

		if (!Paging_MapPage(proc, va, pa, true)) {
			Page_Release(page_ptr, 0);
			break;
		}

		sz += PAGESIZE;
		if (va == va_end) break;

		va += PAGESIZE;
		pa += PAGESIZE;
	}

	return sz;
}

bool Paging_UnmapPages(struct Process* proc, void* virt_addr, size_t size) {
    
    uintptr_t va = PAGEADDRESS(virt_addr + PAGESIZE - 1);
    uintptr_t va_end = PAGEADDRESS(virt_addr + size - 1);

	bool unmapping_failed = false;

    while (true) {
		if (va >= KERNEL_MMAP_VIRTUAL_START) {
			unmapping_failed = true;
			break;
		}
        if (va > va_end) break;
        if (!Paging_UnmapPage(proc, va)) unmapping_failed = true;
        va += PAGESIZE;
    }

    return unmapping_failed;
}

bool Paging_Initialize(struct Process* proc) {

    uintptr_t* pgd = Page_Acquire(0);
    if (pgd == (uintptr_t*)PAGE_LIST_NULL) return false;

    memset(pgd, 0, PAGESIZE);

    for (size_t i = KERNEL_MMAP_VIRTUAL_START; i > 0; i+=PAGING_EXTPAGESIZE) pgd[PAGEDIRIDX(i)] = Kernel_pagedirectory[PAGEDIRIDX(i)];

	proc->task_context->cr3 = pgd;
    return true;
}

void Paging_Terminate(struct Process* proc) {

	Paging_UnmapPages(proc, (void*)0, KERNEL_MMAP_VIRTUAL_START);
	for (size_t i = 0; i < KERNEL_MMAP_VIRTUAL_START; i+=PAGING_EXTPAGESIZE) {
		uintptr_t page_table = KERNADDR(PAGEADDRESS(proc->task_context->cr3[PAGEDIRIDX(i)]));
		Page_Release((void*)page_table, 0);
		proc->task_context->cr3[PAGEDIRIDX(i)] = 0;
	}

	Page_Release(proc->task_context->cr3, 0);
}

bool Paging_SetFlags(struct Process* proc, void* virt_addr, uint16_t flags) {

	if (!Paging_IsPageMapped(proc, (uintptr_t)virt_addr)) return false;

	uintptr_t* page_table = (uintptr_t*)KERNADDR(PAGEADDRESS(proc->task_context->cr3[PAGEDIRIDX(virt_addr)]));
	page_table[PAGETABIDX(virt_addr)] |= flags;
	return true;
}

bool Paging_UnsetFlags(struct Process* proc, void* virt_addr, uint16_t flags) {

    if (!Paging_IsPageMapped(proc, (uintptr_t)virt_addr)) return false;

	uintptr_t* page_table = (uintptr_t*)KERNADDR(PAGEADDRESS(proc->task_context->cr3[PAGEDIRIDX(virt_addr)]));
    page_table[PAGETABIDX(virt_addr)] &= ~flags;
	return true;
}

bool Paging_Clone(struct Process* proc, struct Process* clone_proc) {

	uintptr_t* page_dir = proc->task_context->cr3;
	uintptr_t* clone    = clone_proc->task_context->cr3;

	bool cloning_failed = false;

	for (size_t i = 0; i < PAGEDIRIDX(KERNEL_MMAP_VIRTUAL_START); i++) {
		if (page_dir[i] == 0) continue;

		void* page_table = Page_Acquire(0);
		if (page_table == PAGE_LIST_NULL) {
			cloning_failed = true;
			break;
		}
		clone[i] = PHYSADDR(page_table) + PAGEOFFSET(page_dir[i]);

		uintptr_t* ptab = (uintptr_t*)KERNADDR(PAGEADDRESS(page_dir[i]));
		for (size_t j = 0; j < PAGING_PGTAB_NENTRIES; j++) {
			if (ptab[j] == 0) continue;

			void* page = Page_Acquire(0);
			if (page == PAGE_LIST_NULL) {
				cloning_failed = true;
				break;
			}
			((uintptr_t*)page_table)[j] = PHYSADDR(page) + PAGEOFFSET(ptab[j]);
			
			memmove(page, (void*)KERNADDR(PAGEADDRESS(ptab[j])), PAGESIZE);
		}
		if (cloning_failed) break;
	}

	if (cloning_failed) {
		Paging_Terminate(clone_proc);
		return false;
	}

	return true;
}

uintptr_t Paging_GetHigherHalfAddress(struct Process* proc, uintptr_t user_addr) {

	if (user_addr > KERNEL_MMAP_VIRTUAL_START) return 0;
	if (!Paging_IsPageMapped(proc, user_addr)) return 0;

    uintptr_t* page_table = (uintptr_t*)KERNADDR(PAGEADDRESS(proc->task_context->cr3[PAGEDIRIDX(user_addr)]));
	if ((page_table[PAGETABIDX(user_addr)] & PAGING_PTE_USER) == 0) return 0;

    return KERNADDR(PAGEADDRESS(page_table[PAGETABIDX(user_addr)])) + PAGEOFFSET(user_addr);
}

bool Paging_Copy(struct Process* dest_proc, uintptr_t dest_addr, uintptr_t src_addr, size_t nbytes) {

	bool success = true;

	while (nbytes > 0) {
		uintptr_t dest_hhalf_addr = Paging_GetHigherHalfAddress(dest_proc, dest_addr);
		if (dest_hhalf_addr == 0) {
			success = false;
			break;
		}

		size_t bytes_in_this_page = PAGESIZE - PAGEOFFSET(dest_addr);
		if (nbytes < bytes_in_this_page) bytes_in_this_page = nbytes;

		memmove((void*)dest_hhalf_addr, (void*)src_addr, bytes_in_this_page);

		dest_addr += bytes_in_this_page;
		nbytes -= bytes_in_this_page;
	}

	return success;
}
