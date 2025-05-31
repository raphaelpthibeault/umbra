#ifndef __PARTITION_H__
#define __PARTITION_H__

#include <types.h>
#include <lib/guid.h>
#include <drivers/disk.h>

#define NO_PARTITION  (-1)
#define INVALID_TABLE (-2)
#define END_OF_TABLE  (-3)

struct partition {
	int number;
	uint64_t first_sector;
	uint64_t total_sectors; /* length in number of sectors */
};

int partitions_get(disk_t *disk);

#endif // !__PARTITION_H__
