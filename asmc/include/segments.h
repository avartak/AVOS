#ifndef ASMC_SEGMENTS_H
#define ASMC_SEGMENTS_H

// We define the asm code to be "volatile"
// This is a simple fencing against optimizations of the compiler
// It's less severe than further adding a "memory" clobber
// For now we do it this way
 


// Loading segment registers for kernel and user modes

static inline void LoadKernelSegments() {

	asm volatile (
		"
		movw  $0x08, %%ax;
		movw  %%ax , %%cs;	
		movw  $0x10, %%ax;
		movw  %%ax , %%ds;	
		movw  %%ax , %%es;	
		movw  %%ax , %%fs;	
		movw  %%ax , %%gs;	
		movw  $0x18, %%eax;
		movw  %%eax, %%ss;	
		"
		:
		:
		: "%eax" 
	)

}

static inline void LoadUserSegments() {

    asm volatile (
		"
		movw  $0x20, %%ax;
		movw  %%ax , %%cs;	
		movw  $0x28, %%ax;
		movw  %%ax , %%ds;	
		movw  %%ax , %%es;	
		movw  %%ax , %%fs;	
		movw  %%ax , %%gs;	
		movw  $0x30, %%eax;
		movw  %%eax, %%ss;	
        "
        :
        :
        : "%eax"
    )

}	

#endif
