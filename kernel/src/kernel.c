#include <asmc/include/interrupts.h>
#include <kernel/include/defs.h>

#include <stdint.h>

void SetupInitialKernelPaging();
void ShowWelcomeScreen();

void Kmain() {

	// Clear interrupts
	ClearInterrupts();


	// Remove identity map of the first 4 MB of physical memory -- from now on, kernel is exclusively higher half ; Map 4 MB - 8 MB physical memory, reserved for page maps, to higher half
	SetupInitialKernelPaging();

	// Show welcome screen
	ShowWelcomeScreen();
	
    return;
}

void SetupInitialKernelPaging() {

    // We will now clear the identity map
    uint32_t* page_directory = (uint32_t*)LOC_PAGE_DIRECTORY_VM;
    page_directory[0] = 0;

    // We will reserve physical memory 4 MB - 8 MB for page tables and map it to higher half ; The corresponding page table is put right after the kernel page table in memory (at 0x12000)
    uint32_t* page_map = (uint32_t*)LOC_PAGEMAP_TABLE_VM;

    uint32_t i;
    for(i = 0; i < 1024; i++) {
        page_map[i] = (i * 0x1000 + LOC_PAGEMAP_PM) | 3; // attributes: supervisor level, read/write, present.
    }   
    page_directory[769] = ((uint32_t)page_map) | 3;

}

void ShowWelcomeScreen() {

    // Video buffer -- now at 0xC00B8000
    char* screen = (char*)0xC00B8000;

    // Clear screen
    unsigned int i = 0;

    while (i < 80*25*2) {
        screen[i] = 0;
        i++;
    }

    // Welcome string
    char str[] = "Welcome to AVOS!";


    // Print message to screen -- the magic number 64 causes the message to start on line 0 and 32 character offset
    i = 0;
    while (str[i] != 0) {
        screen[64 + 2*i]   = str[i];
        screen[64 + 2*i+1] = 0x04;
        i++;
    }

    return;

}
