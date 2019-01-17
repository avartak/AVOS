#ifndef X86_KERNEL_PAGING_H
#define X86_KERNEL_PAGING_H

static inline void EnablePGBitInCR0();
static inline void LoadPageDirectory(uint32_t* pdt);
void InitPaging();

#endif
