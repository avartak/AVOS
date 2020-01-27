#include <kernel/arch/initial/include/initialize.h>
#include <kernel/arch/i386/include/controlregs.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/arch/i386/include/functions.h>
#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/acpi/include/madt.h>
#include <kernel/arch/timer/include/pit.h>
#include <kernel/arch/keyboard/include/keyboard.h>
#include <kernel/arch/console/include/console.h>
#include <kernel/clib/include/string.h>

size_t   Kernel_numcpus_online = 0; 
uint32_t Kernel_pagedirectory[X86_PAGING_PGDIR_NENTRIES]__attribute__((aligned(X86_PAGING_PAGESIZE)));

struct X86_GDT_Entry      Kernel_GDT[7];
struct X86_GDT_Descriptor Kernel_GDT_desc;

struct X86_IDT_Entry      Kernel_IDT[0x100];
struct X86_IDT_Descriptor Kernel_IDT_desc;

void Welcome() {

    for (size_t i = 0; i < CONSOLE_VGA_NUM_COLUMNS; i++) Console_screen[i] = Console_Attribute(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_GREEN) | 0;

    char* str = "Welcome to AVOS!";
    for (size_t i = 0; str[i] != 0; i++) Console_screen[CONSOLE_POS_START_WELCOME+i] = Console_Attribute(CONSOLE_COLOR_RED, CONSOLE_COLOR_GREEN) | str[i];

    Console_ClearScreen();
    Console_SetCursorPosition(Console_pos);
    Console_MakeCursorVisible();

}

void Initialize_BSP_Paging() {

	uintptr_t ipd = (uintptr_t)Kernel_pagedirectory - KERNEL_HIGHER_HALF_OFFSET;
	uint32_t*  pd = (uint32_t*)ipd;
	
	pd[0] = 0 | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE;
	for (size_t i = KERNEL_MMAP_VIRTUAL_START; i < KERNEL_MMAP_VIRTUAL_END; i+=X86_PAGING_EXTPAGESIZE) {
		pd[X86_PAGING_PGDIR_IDX(i)] = (i - KERNEL_HIGHER_HALF_OFFSET) | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE;
	}
	for (size_t i = KERNEL_MMAP_HIMEMIO_START; i > 0; i+=X86_PAGING_EXTPAGESIZE) {
		pd[X86_PAGING_PGDIR_IDX(i)] = i | X86_PAGING_PDE_PRESENT | X86_PAGING_PDE_READWRITE | X86_PAGING_PDE_PSE;
	}
	
	X86_CR3_Write(ipd);
	X86_CR4_Write(X86_CR4_Read() | X86_CR4_PSE);
	X86_CR0_Write(X86_CR0_Read() | X86_CR0_PG | X86_CR0_WP);

}

void Initialize_AP_Paging() {

    uintptr_t ipd = (uintptr_t)Kernel_pagedirectory - KERNEL_HIGHER_HALF_OFFSET;
   
    X86_CR3_Write(ipd);
    X86_CR4_Write(X86_CR4_Read() | X86_CR4_PSE);
    X86_CR0_Write(X86_CR0_Read() | X86_CR0_PG | X86_CR0_WP);

}


void Initialize_BSP_GDT() {

	uint8_t gdt_kern_code_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING0 | X86_GDT_FLAGS_CODEORDATA | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_user_code_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING3 | X86_GDT_FLAGS_CODEORDATA | X86_GDT_FLAGS_EXECUTABLE | X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_kern_data_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING0 | X86_GDT_FLAGS_CODEORDATA |                            X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_user_data_access = X86_GDT_FLAGS_PRESENT | X86_GDT_FLAGS_PRIV_RING3 | X86_GDT_FLAGS_CODEORDATA |                            X86_GDT_FLAGS_READWRITE;
	uint8_t gdt_flags            = X86_GDT_FLAGS_4GB     | X86_GDT_FLAGS_PMODE32;
	
	X86_GDT_SetupEntry(&(Kernel_GDT[0]), 0, 0                   , 0                   , 0        );
	X86_GDT_SetupEntry(&(Kernel_GDT[1]), 0, X86_GDT_SEGLIMIT_4GB, gdt_kern_code_access, gdt_flags);
	X86_GDT_SetupEntry(&(Kernel_GDT[2]), 0, X86_GDT_SEGLIMIT_4GB, gdt_kern_data_access, gdt_flags);
	X86_GDT_SetupEntry(&(Kernel_GDT[3]), 0, X86_GDT_SEGLIMIT_4GB, gdt_user_code_access, gdt_flags);
	X86_GDT_SetupEntry(&(Kernel_GDT[4]), 0, X86_GDT_SEGLIMIT_4GB, gdt_user_data_access, gdt_flags);
	
	Kernel_GDT_desc.limit = (sizeof(struct X86_GDT_Entry))*7 - 1;
	Kernel_GDT_desc.base  = (uintptr_t)Kernel_GDT;
	
	X86_GDT_Load(&Kernel_GDT_desc);
	X86_GDT_LoadKernelSegments();
}

void Initialize_AP_GDT() {

    X86_GDT_Load(&Kernel_GDT_desc);
    X86_GDT_LoadKernelSegments();
}


void Initialize_BSP_IDT() {

	for (size_t i = 0; i < 256; i++) X86_IDT_SetupEntry(&(Kernel_IDT[i]), 0, 0, 0);

	Kernel_IDT_desc.limit = (sizeof(struct X86_IDT_Entry))*0x100 - 1;
	Kernel_IDT_desc.base  = (uintptr_t)Kernel_IDT;
	
	X86_IDT_Load(&Kernel_IDT_desc);
}

void Initialize_AP_IDT() {

    X86_IDT_Load(&Kernel_IDT_desc);
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

	while (Kernel_numcpus_online == inumcpus);
    return true;
}


void Initialize_BSP() {

    Initialize_BSP_GDT();
    Initialize_BSP_IDT();

    LocalAPIC_Initialize();

    Console_Print("CPU (%x) initialized\n", LocalAPIC_ID());
    Kernel_numcpus_online++;

}

void Initialize_AP() {

	Initialize_AP_GDT();
	Initialize_AP_IDT();

	LocalAPIC_Initialize();

    Console_Print("CPU (%x) initialized\n", LocalAPIC_ID());
	Kernel_numcpus_online++;

	while (true) X86_Halt();
}

void Initialize_System() {

    IOAPIC_Initialize();

    KERNEL_IDT_ADDENTRY(0x20);
    PIT_Initialize();

    KERNEL_IDT_ADDENTRY(0x21);
    Keyboard_Initialize();

    extern uintptr_t StartAP;
    memmove((void*)(KERNEL_AP_BOOT_START_ADDR+KERNEL_HIGHER_HALF_OFFSET), &StartAP, 0x1000);
    for (size_t i = 0; i < LocalAPIC_Num; i++) {
        struct MADT_Entry_LocalAPIC* lapic_id = (struct MADT_Entry_LocalAPIC*)(LocalAPIC_InfoPtrs[i]);
        if (LocalAPIC_ID() == lapic_id->apic_id) continue;
        Initialize_CPU(lapic_id->apic_id, KERNEL_AP_BOOT_START_ADDR);
    }

    Console_Print("CPUs online : %x\n", Kernel_numcpus_online);

    Kernel_pagedirectory[0] = 0;

	while (true) X86_Halt();
}