#include <kernel/arch/initialize/include/initialize.h>
#include <kernel/arch/processor/include/gdt.h>
#include <kernel/arch/processor/include/idt.h>
#include <kernel/arch/processor/include/controlregs.h>
#include <kernel/arch/processor/include/ioports.h>
#include <kernel/arch/processor/include/functions.h>
#include <kernel/arch/paging/include/paging.h>
#include <kernel/arch/tasking/include/cpu.h>
#include <kernel/arch/apic/include/lapic.h>
#include <kernel/arch/apic/include/ioapic.h>
#include <kernel/arch/acpi/include/madt.h>
#include <kernel/devices/timer/include/pit.h>
#include <kernel/devices/keyboard/include/keyboard.h>
#include <kernel/devices/console/include/console.h>
#include <kernel/core/setup/include/setup.h>
#include <kernel/core/multiboot/include/multiboot.h>
#include <kernel/core/memory/include/physmem.h>
#include <kernel/core/taskmaster/include/state.h>
#include <kernel/core/taskmaster/include/scheduler.h>
#include <kernel/core/synch/include/spinlock.h>
#include <kernel/core/synch/include/irqlock.h>
#include <kernel/clib/include/string.h>

size_t   Kernel_numcpus_online = 0; 
//size_t   Kernel_stack_offset   = sizeof(struct State) + sizeof(struct CPU) + sizeof(struct KProc);
size_t   Kernel_stack_offset   = (KERNEL_STACK_SIZE - (sizeof(struct State) + sizeof(struct CPU) + sizeof(struct KProc))) & ~(KERNEL_STACK_ALIGNMENT-1);

uint32_t Kernel_pagedirectory[X86_PAGING_PGDIR_NENTRIES]__attribute__((aligned(X86_PAGING_PAGESIZE))) = {
	[0]                                               = (0) | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE,
	[X86_PAGING_PGDIR_IDX(KERNEL_MMAP_VIRTUAL_START)] = (0) | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE
};

void Initialize_Memory() {

	/* Paging tables */
	
	uint32_t*  pd = (uint32_t*)PHYSADDR(Kernel_pagedirectory);
	
	for (size_t i = X86_PAGING_EXTPAGESIZE; i < KERNEL_MMAP_VIRTUAL_START; i+=X86_PAGING_EXTPAGESIZE) {
		pd[X86_PAGING_PGDIR_IDX(i)] = 0;
	}
	for (size_t i = KERNEL_MMAP_VIRTUAL_START; i < KERNEL_MMAP_VIRTUAL_END; i+=X86_PAGING_EXTPAGESIZE) {
		pd[X86_PAGING_PGDIR_IDX(i)] = (i - KERNEL_HIGHER_HALF_OFFSET) | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE;
	}
	for (size_t i = KERNEL_MMAP_HIMEMIO_START; i > 0; i+=X86_PAGING_EXTPAGESIZE) {
		pd[X86_PAGING_PGDIR_IDX(i)] = i | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE;
	}
	
	/* Page allocation tables */
	
	Page_BuddyMaps_Initialize();
}

bool Initialize_CPU(uint8_t local_apic_id, uint32_t boot_address) {

	size_t inumcpus = Kernel_numcpus_online;

	/* Here we try to follow the exact procedure given in the Intel MP specification (1997) */
	
	/* Make sure the INIT IPI causes a warm reset */
	
	// BIOS POST procedure performs a warm reset if it reads the shutdown code 0x0A from the CMOS RAM location 0xF - so write 0xA to CMOS RAM location 0xF
	X86_Outb(SMP_CMOS_RAM_ADDR_PORT, SMP_WARM_RESET_ADDR);
	X86_Outb(SMP_CMOS_RAM_DATA_PORT, SMP_WARM_RESET_SIGN);
	
	// Set the warm reset vector [40:67] containing the (real mode long) address of the boot code - this is the code that the CPU will start executing after a warm reset
	uint16_t* warm_reset_vector = (uint16_t*)(SMP_WARM_RESET_VECTOR + KERNEL_HIGHER_HALF_OFFSET);
	warm_reset_vector[0] = SMP_WARM_RESET_BOOT_OFF(boot_address);
	warm_reset_vector[1] = SMP_WARM_RESET_BOOT_SEG(boot_address);

	PIT_Set(10000);

	// Issue an INIT-LEVEL-ASSERT followed by INIT-LEVEL-DEASSERT (with a 200 microsecond delay)
	LocalAPIC_WriteTo(LAPIC_REG_ICRHI, local_apic_id << 24);
	LocalAPIC_WriteTo(LAPIC_REG_ICRLO, LAPIC_ICR_DELIVERY_INIT | LAPIC_ICR_TRIGGER_LEVEL | LAPIC_ICR_LEVEL_ASSERT);
	PIT_Delay(2);
	LocalAPIC_WriteTo(LAPIC_REG_ICRLO, LAPIC_ICR_DELIVERY_INIT | LAPIC_ICR_TRIGGER_LEVEL | LAPIC_ICR_LEVEL_DEASSERT);
	PIT_Delay(2);
	
	// Issue the STARTUP IPI twice (second one should be ignored if the first one succeeds)
	LocalAPIC_WriteTo(LAPIC_REG_ICRHI, local_apic_id << 24);
	LocalAPIC_WriteTo(LAPIC_REG_ICRLO, LAPIC_ICR_DELIVERY_STARTUP | LAPIC_STARTUP_VECTOR(boot_address));
	PIT_Delay(2);
	
	LocalAPIC_WriteTo(LAPIC_REG_ICRHI, local_apic_id << 24);
	LocalAPIC_WriteTo(LAPIC_REG_ICRLO, LAPIC_ICR_DELIVERY_STARTUP | LAPIC_STARTUP_VECTOR(boot_address));
	PIT_Delay(2);

	PIT_Reset();

	while (Kernel_numcpus_online == inumcpus);
	
	return true;
}


void Initialize_ThisProcessor() {

	/* Paging */
	
	X86_CR3_Write(PHYSADDR(Kernel_pagedirectory));
	X86_CR4_Write(X86_CR4_Read() | X86_CR4_PSE);
	X86_CR0_Write(X86_CR0_Read() | X86_CR0_PG | X86_CR0_WP);
	
	/* State of the base kernel thread */
	
	struct State* state       = STATE_CURRENT;
	state->preemption_vetos   = 0;
	state->interrupt_priority = 0;
	state->process            = (struct Process*)0;
	state->kernel_task        = (struct KProc*)((uintptr_t)state - sizeof(struct CPU) - sizeof(struct KProc));
	state->kernel_task->cpu   = (struct CPU*)((uintptr_t)state - sizeof(struct CPU));
	
	/* State of the CPU */
	
	struct CPU* cpu = state->kernel_task->cpu;
	cpu->apic_id    = LocalAPIC_ID();
	cpu->acpi_id    = 0;
	
	/* GDT */
	
	uint32_t tss_seg_base  = (uint32_t)(&(cpu->task_state));
	uint32_t tss_seg_limit = sizeof(struct X86_TSS) - 1;
	
	uint8_t gdt_kern_code_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING0 | X86_GDT_FLAGS_CODEORDATA | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_user_code_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING3 | X86_GDT_FLAGS_CODEORDATA | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_kern_data_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING0 | X86_GDT_FLAGS_CODEORDATA |                            X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_user_data_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING3 | X86_GDT_FLAGS_CODEORDATA |                            X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_flags            = X86_GDT_FLAGS_4GB     | X86_GDT_FLAGS_PMODE32;
	uint8_t gdt_tss_access       = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_ACCESSED; 
	   
	X86_GDT_SetupEntry(&(cpu->gdt[X86_GDT_ENTRY_NULL])     , 0           , 0                   , 0                   , 0        );
	X86_GDT_SetupEntry(&(cpu->gdt[X86_GDT_ENTRY_KERN_CODE]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_kern_code_access, gdt_flags);
	X86_GDT_SetupEntry(&(cpu->gdt[X86_GDT_ENTRY_KERN_DATA]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_kern_data_access, gdt_flags);
	X86_GDT_SetupEntry(&(cpu->gdt[X86_GDT_ENTRY_USER_CODE]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_user_code_access, gdt_flags);
	X86_GDT_SetupEntry(&(cpu->gdt[X86_GDT_ENTRY_USER_DATA]), 0           , X86_GDT_SEGLIMIT_4GB, gdt_user_data_access, gdt_flags);
	X86_GDT_SetupEntry(&(cpu->gdt[X86_GDT_ENTRY_TSS])      , tss_seg_base, tss_seg_limit       , gdt_tss_access      , 0        );   
	
	cpu->gdt_desc.limit = (sizeof(struct X86_GDT_Entry))*X86_GDT_NENTRIES - 1;
	cpu->gdt_desc.base  = (uintptr_t)(cpu->gdt);
	
	X86_GDT_Load(&(cpu->gdt_desc));
	X86_GDT_LoadKernelSegments();
	X86_GDT_LoadTaskRegister(X86_GDT_SEG_TSS | X86_GDT_RPL3);	
	
	/* IDT */
	
	for (size_t i = 0; i < X86_IDT_NENTRIES; i++) X86_IDT_SetupEntry(&(cpu->idt[i]), 0, 0, 0);

	X86_IDT_SetupEntry(&(cpu->idt[0x20]), (uintptr_t)Interrupt_0x20, X86_GDT_SEG_KERN_CODE, X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL0 | X86_IDT_TYPE_INTR32);
	X86_IDT_SetupEntry(&(cpu->idt[0x21]), (uintptr_t)Interrupt_0x21, X86_GDT_SEG_KERN_CODE, X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL0 | X86_IDT_TYPE_INTR32);
	X86_IDT_SetupEntry(&(cpu->idt[0x30]), (uintptr_t)Interrupt_0x30, X86_GDT_SEG_KERN_CODE, X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL0 | X86_IDT_TYPE_INTR32);
	X86_IDT_SetupEntry(&(cpu->idt[0x80]), (uintptr_t)Interrupt_0x80, X86_GDT_SEG_KERN_CODE, X86_IDT_FLAGS_PRESENT | X86_IDT_FLAGS_DPL3 | X86_IDT_TYPE_TRAP32);

	cpu->idt_desc.limit = (sizeof(struct X86_IDT_Entry))*X86_IDT_NENTRIES - 1;
	cpu->idt_desc.base  = (uintptr_t)(cpu->idt);
	
	X86_IDT_Load(&(cpu->idt_desc));

	/* TSS */
	
    cpu->task_state.ss0 = X86_GDT_SEG_KERN_DATA;
    cpu->task_state.iomap_base_address = (uint16_t)0xFFFF;

	/* Local APIC */
	
	LocalAPIC_Initialize();
	
	/* CPU online */
	
	Kernel_numcpus_online++;
}

void Initialize_System() {

	IOAPIC_Initialize();
	
	Console_Initialize();
	
	PIT_Initialize(PIT_IRQLINE, 0x20);
	
	Keyboard_Initialize(KEYBOARD_PS2_IRQLINE, 0x21);
	
	Scheduler_Initialize();

	extern uintptr_t StartAP;
	extern uintptr_t EndAP;
	memmove((void*)(KERNEL_AP_BOOT_START_ADDR+KERNEL_HIGHER_HALF_OFFSET), &StartAP, (uintptr_t)(&EndAP) - (uintptr_t)(&StartAP));
	for (size_t i = 0; i < LocalAPIC_Num; i++) {
		struct MADT_Entry_LocalAPIC* lapic_id = (struct MADT_Entry_LocalAPIC*)(LocalAPIC_InfoPtrs[i]);
		if (LocalAPIC_ID() == lapic_id->apic_id) continue;
		Initialize_CPU(lapic_id->apic_id, KERNEL_AP_BOOT_START_ADDR);
	}
	
	Kernel_pagedirectory[0] = 0;

}

void GetToWork() {

	Console_Print("CPU %u starting work\n", LocalAPIC_ID());

	LocalAPIC_Initialize_Timer(0x30, 200);

	Schedule();
}
