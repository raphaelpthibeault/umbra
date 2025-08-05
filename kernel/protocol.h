#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <types.h>

#define MEMORY_MAP_MEMORY_AVAILABLE              1
#define MEMORY_MAP_MEMORY_RESERVED               2
#define MEMORY_MAP_MEMORY_ACPI_RECLAIMABLE       3
#define MEMORY_MAP_MEMORY_NVS                    4
#define MEMORY_MAP_MEMORY_BADRAM                 5

struct memory_map_entry
{
	uint64_t base;
	uint64_t length;
	uint32_t type;	
	uint32_t zero;
} __attribute__((packed));

struct memory_map
{
	uint64_t entry_count;
	struct memory_map_entry *entries;
};

#endif // !__PROTOCOL_H__
