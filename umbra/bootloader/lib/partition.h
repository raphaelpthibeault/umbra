#ifndef __PARTITION_H__
#define __PARTITION_H__

#include <types.h>
#include <lib/guid.h>

struct volume {
	uint8_t drive; // should be 0x80	
	size_t fastest_xfer_size;
	uint8_t idx;

	bool is_optical;
	bool pxe;

	uint8_t partition;
	uint8_t sector_size;
	struct volume *backing_dev;
	int max_partition;
	int cache_status;
	uint8_t *cache;
	uint64_t cached_block;
	uint64_t first_sector;
	uint64_t sector_count;

	bool guid_valid;
	struct guid guid;
	bool partition_guid_valid;
	struct guid partition_guid;

	bool fslabel_valid;
	char *fslabel;
};

#endif // !__PARTITION_H__
