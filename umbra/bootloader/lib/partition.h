#ifndef __PARTITION_H__
#define __PARTITION_H__

#include <types.h>
#include <lib/guid.h>

typedef struct partition_map {
	struct partition_map *next;
	struct partition_map **prev;
	const char *name;
} partition_map_t;

struct partition {
	int number;
	uint64_t start; /* start relative to parent */
	uint64_t length; /* length in number of sectors */
	uint64_t offset; /* offset of the partition table */
	int index; /* index in partition table */
	struct partition *parent; /* parent partition (physically contains this partition) */
	partition_map_t partmap;
};

#endif // !__PARTITION_H__
