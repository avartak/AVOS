#include <kernel/initial/include/initialize.h>
#include <kernel/arch/include/controlregs.h>

uint32_t Kernel_pagedirectory[X86_PAGING_PGDIR_NENTRIES]__attribute__((aligned(X86_PAGING_PAGESIZE))) = {
	[0] = (0) | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE,
};

struct X86_GDT_Entry      Kernel_GDT[6];
struct X86_GDT_Descriptor Kernel_GDT_desc;
struct X86_TSS            Kernel_TSS;

struct X86_IDT_Entry      Kernel_IDT[0x100];
struct X86_IDT_Descriptor Kernel_IDT_desc;

void Initialize_Paging() {

	uintptr_t ipd = (uintptr_t)Kernel_pagedirectory - KERNEL_HIGHER_HALF_OFFSET;
	uint32_t*  pd = (uint32_t*)ipd;

	for (size_t i = KERNEL_MMAP_VIRTUAL_START; i < KERNEL_MMAP_VIRTUAL_END; i+=MEMORY_SIZE_PAGE_PSE) {
		pd[X86_PAGING_PGDIR_IDX(i)] = (i - KERNEL_HIGHER_HALF_OFFSET) | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE;
	}
	for (size_t i = KERNEL_MMAP_VIRTUAL_END; i > 0; i+=MEMORY_SIZE_PAGE_PSE) {
		pd[X86_PAGING_PGDIR_IDX(i)] = i | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE;
	}

	X86_CR3_Write(ipd);
	X86_CR4_Write(X86_CR4_Read() | X86_CR4_PSE);
	X86_CR0_Write(X86_CR0_Read() | X86_CR0_PG | X86_CR0_WP);
}

void Initialize_GDT() {

	Kernel_TSS.ss0 = X86_GDT_SEG_KERN_DATA;
	Kernel_TSS.esp = KERNEL_HIGHER_HALF_OFFSET + 0x400000;
	for (size_t i = 0; i < 0x2000; i++) Kernel_TSS.ioport_map[i] = 0xFF;
	
	uint16_t tss_seg_desc  = X86_GDT_SEG_USER_TSS | X86_GDT_RPL3;
	uint32_t tss_seg_base  = (uint32_t)(&Kernel_TSS) - KERNEL_HIGHER_HALF_OFFSET;
	uint32_t tss_seg_limit = sizeof(Kernel_TSS) - 1;

	uint8_t gdt_kern_code_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING0 | X86_GDT_FLAGS_CODEORDATA | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_user_code_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING3 | X86_GDT_FLAGS_CODEORDATA | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_kern_data_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING0 | X86_GDT_FLAGS_CODEORDATA |                            X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_user_data_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING3 | X86_GDT_FLAGS_CODEORDATA |                            X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_flags            = X86_GDT_FLAGS_4GB     | X86_GDT_FLAGS_PMODE32;
	uint8_t tss_access           = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_ACCESSED;

	X86_GDT_SetupEntry(&(Kernel_GDT[0]), 0           , 0                   , 0                   , 0        );
	X86_GDT_SetupEntry(&(Kernel_GDT[1]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_kern_code_access, gdt_flags);
	X86_GDT_SetupEntry(&(Kernel_GDT[2]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_kern_data_access, gdt_flags);
	X86_GDT_SetupEntry(&(Kernel_GDT[3]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_user_code_access, gdt_flags);
	X86_GDT_SetupEntry(&(Kernel_GDT[4]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_user_data_access, gdt_flags);
	X86_GDT_SetupEntry(&(Kernel_GDT[5]), tss_seg_base, tss_seg_limit       , tss_access          , 0        );
	
	Kernel_GDT_desc.limit = (sizeof(struct X86_GDT_Entry))*6 - 1;
	Kernel_GDT_desc.base  = (uintptr_t)Kernel_GDT;
	
	X86_GDT_Load(&Kernel_GDT_desc);
	X86_GDT_LoadKernelSegments();
	X86_GDT_LoadTaskRegister(tss_seg_desc);
	
	return;

}

void Initialize_IDT() {

    for (size_t i = 0; i < 256; i++) X86_IDT_SetupEntry(&(Kernel_IDT[i]), 0, 0, 0);

	/* Example to add interrupts to the IDT

    uint8_t type_intr = X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL0 | X86_IDT_TYPE_INTR32;
    uint8_t type_task = X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL3 | X86_IDT_TYPE_TASK;

    X86_IDT_SetupEntry(&(Kernel_IDT[0x00]), (uintptr_t)Interrupt_0x0 , X86_GDT_SEG_KERN_CODE, type_intr);
    X86_IDT_SetupEntry(&(Kernel_IDT[0x80]), (uintptr_t)Interrupt_0x80, X86_GDT_SEG_KERN_CODE, type_task);
	*/

    Kernel_IDT_desc.limit = (sizeof(struct X86_IDT_Entry))*0x100 - 1;
    Kernel_IDT_desc.base  = (uintptr_t)Kernel_IDT;

    X86_IDT_Load(&Kernel_IDT_desc);

    return;

}

