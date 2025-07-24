#ifndef __PMM_H__
#define __PMM_H__

#include <types.h>
#include <lib/misc.h>

#define PAGE_SIZE 0x1000

#define MEMMAP_USABLE                 1
#define MEMMAP_RESERVED               2
#define MEMMAP_ACPI_RECLAIMABLE       3
#define MEMMAP_ACPI_NVS               4
#define MEMMAP_BAD_MEMORY             5
#define MEMMAP_BOOTLOADER_RECLAIMABLE 0x1000
#define MEMMAP_KERNEL_AND_MODULES			0x1001

#define ALIGN_UP(x, a) ({ \
    __auto_type ALIGN_UP_value = (x); \
    __auto_type ALIGN_UP_align = (a); \
    ALIGN_UP_value = DIV_ROUNDUP(ALIGN_UP_value, ALIGN_UP_align) * ALIGN_UP_align; \
    ALIGN_UP_value; \
})

#define ALIGN_DOWN(x, a) ({ \
    __auto_type ALIGN_DOWN_value = (x); \
    __auto_type ALIGN_DOWN_align = (a); \
    ALIGN_DOWN_value = (ALIGN_DOWN_value / ALIGN_DOWN_align) * ALIGN_DOWN_align; \
    ALIGN_DOWN_value; \
})

#define MAX_MEMMAP_ENTRIES 256

struct memmap_entry 
{
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t acpi;
};

struct memory_info
{
	size_t upper_memory;	
	size_t lower_memory;
};

extern char _bss_start[];
extern char _bss_end[];

extern struct memmap_entry memmap[];
extern size_t memmap_entries;

void memmap_init(void);
void memmap_sanitize_entries(struct memmap_entry *map, size_t *_count, bool align);
bool memmap_alloc_range(uint64_t base, uint64_t length, uint32_t type, uint32_t overlay_type, bool do_panic, bool create_new_entry);
void memmap_free(void *ptr, size_t count);
void *memmap_realloc(void *oldptr, size_t oldsize, size_t newsize); 

void *ext_mem_alloc(size_t count);
void *ext_mem_alloc_aligned(size_t count, uint32_t type, size_t alignment, bool allow_high_alloc);

bool check_usable_memory(uint64_t base, uint64_t top);
struct memmap_entry *get_raw_memmap(size_t *entry_count);
struct memory_info get_mmap_info(size_t mmap_count, struct memmap_entry *mmap);

#endif // !__PMM_H__
