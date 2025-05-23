#ifndef __PMM_H__
#define __PMM_H__

#include <types.h>

#define MEMMAP_USABLE                 1
#define MEMMAP_RESERVED               2
#define MEMMAP_ACPI_RECLAIMABLE       3
#define MEMMAP_ACPI_NVS               4
#define MEMMAP_BAD_MEMORY             5
#define MEMMAP_BOOTLOADER_RECLAIMABLE 0x1000

struct memmap_entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t acpi;
};

extern char _bss_start[];
extern char _bss_end[];

extern struct memmap_entry memmap[];
extern size_t memmap_entries;

void memmap_init(void);
void memmap_sanitize_entries(struct memmap_entry *map, size_t *_count, bool align);
bool memmap_alloc_range(uint64_t base, uint64_t length, uint32_t type, uint32_t overlay_type, bool do_panic, bool create_new_entry);

void *ext_mem_alloc(size_t count);
void *ext_mem_alloc_aligned(size_t count, uint32_t type, size_t alignment, bool allow_high_alloc);


#endif // !__PMM_H__
