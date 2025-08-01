#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <types.h>

#define MEMORY_MAP_FREE     0x00
#define MEMORY_MAP_BUSY     0x01
#define MEMORY_MAP_MMIO     0x02
#define MEMORY_MAP_NOUSE    0x03

struct memory_map_entry
{
	uint64_t base;
	uint64_t length;
	uint64_t signal;
} __attribute__((packed));

struct memory_map
{
	uint64_t entry_count;
	struct memory_map_entry *entries;
};

#endif // !__PROTOCOL_H__
