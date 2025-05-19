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
	uint32_t ACPI;
};

extern struct memmap_entry memmap[];
extern size_t memmap_entries;

void init_memmap(void);

#endif // !__PMM_H__
