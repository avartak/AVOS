#include <boot/general/include/boot.h>

uint32_t Boot_MemoryBlockAboveAddress(uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size) {

    if (addr == BOOT_32BIT_MEMORY_LIMIT || addr + size < addr) return BOOT_32BIT_MEMORY_LIMIT;
   
    uint32_t addr_limit = BOOT_32BIT_MEMORY_LIMIT;
    uint32_t mem = addr_limit;
   
    uint32_t aligned_addr = (addr % align == 0 || align <= 1 ? addr : align * (1 + addr/align));
   
    if (mmap_size == 0 || mmap == MEMORY_NULL_PTR) return mem;
    for (size_t i = 0; i < mmap_size; i++) {
        if (mmap[i].address >= BOOT_32BIT_MEMORY_LIMIT) continue;

        uint32_t mmap_entry_base = (uint32_t)mmap[i].address;
        uint32_t mmap_entry_size = mmap[i].address + mmap[i].size > BOOT_32BIT_MEMORY_LIMIT ? (BOOT_32BIT_MEMORY_LIMIT - mmap[i].address) + 1 : (uint32_t)mmap[i].size;
        if (mmap_entry_size < size) continue;

        uint32_t aligned_base = (mmap_entry_base % align == 0 || align <= 1 ? mmap_entry_base : align * (1 + mmap_entry_base/align));
        if (aligned_base >= mmap_entry_base + mmap_entry_size) continue;
        uint64_t aligned_size = mmap_entry_base + mmap_entry_size - aligned_base;

        if (mmap_entry_base <= aligned_addr && mmap_entry_base + mmap_entry_size >= aligned_addr + size) return aligned_addr;
        if (aligned_base >= addr && aligned_size >= size && aligned_base < mem) mem = aligned_base;
    }
    return mem;
}

uint32_t Boot_MemoryBlockBelowAddress(uint32_t addr, uint32_t size, uint32_t align, struct Boot_Block64* mmap, size_t mmap_size) {

    uint32_t mem = 0;
    uint32_t aligned_addr = (addr % align == 0 || align <= 1 ? addr - size + 1 : align * ((addr - size + 1)/align));

    if (mmap_size == 0 || mmap == MEMORY_NULL_PTR) return mem;
    for (size_t i = 0; i < mmap_size; i++) {
        if (mmap[i].address >= BOOT_32BIT_MEMORY_LIMIT) continue;

        uint32_t mmap_entry_base = (uint32_t)mmap[i].address;
        uint32_t mmap_entry_size = mmap[i].address + mmap[i].size > BOOT_32BIT_MEMORY_LIMIT ? (BOOT_32BIT_MEMORY_LIMIT - mmap[i].address) + 1 : (uint32_t)mmap[i].size;
        if (mmap_entry_size < size) continue;

        uint32_t shifted_base = mmap_entry_base + mmap_entry_size - size;
        uint32_t aligned_base = (shifted_base % align == 0 || align <= 1 ? aligned_base : align * (1 + shifted_base/align));
        if (aligned_base >= mmap_entry_base + mmap_entry_size) continue;
        uint64_t aligned_size = mmap_entry_base + mmap_entry_size - aligned_base;

        if (mmap_entry_base <= aligned_addr && mmap_entry_base + mmap_entry_size >= aligned_addr + size) return aligned_addr;
        if (aligned_base <= addr - size + 1 && aligned_size >= size && aligned_base > mem) mem = aligned_base;
    }
    if (mem != 0) return mem;
    else return BOOT_32BIT_MEMORY_LIMIT;
}

