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
	disk_t *parent_disk;
	char *fslabel;
	bool fslabel_valid;
};

int partitions_get(disk_t *disk);
void partition_read(struct partition *part, size_t loc, size_t size, void *buf);

#endif // !__PARTITION_H__
