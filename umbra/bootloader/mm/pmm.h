#ifndef __PMM_H__
#define __PMM_H__

#include <types.h>

struct memmap_entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t ACPI;
};

#endif // !__PMM_H__
