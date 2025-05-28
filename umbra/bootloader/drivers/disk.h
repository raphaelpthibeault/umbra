#ifndef __DISK_H__
#define __DISK_H__

#include <types.h>
#include <lib/partition.h>

#define FLAG_LBA	1


/* disk and drive are used interchangeably
 * drive: "disk drive"
 * drive_device: physical storage device
 * */

enum DISK_DEVICE_TYPE {
	BIOS_DISK,
};

typedef struct disk_device {
	const char *name; /* device name */	
	enum DISK_DEVICE_TYPE id; /* device id */
} disk_device_t;

typedef struct disk {
	int drive;
	disk_device_t dev; /* underlying device */
	uint64_t total_sectors;
	struct partition *partition;
} disk_t;


void disk_create_index(void);
disk_t *disk_get_by_drive(uint16_t drive);

#endif // !__DISK_H__
