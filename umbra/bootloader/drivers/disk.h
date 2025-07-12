#ifndef __DISK_H__
#define __DISK_H__

#include <types.h>

struct partition;

#define FLAG_LBA	1


/* disk and drive are used interchangeably
 * drive: "disk drive"
 * drive_device: physical storage device
 * */

enum DISK_DEVICE_TYPE {
	BIOS_DISK,
};

struct bios_drive_params {
	uint16_t buf_size;
	uint16_t info_flags;
	uint32_t cylinders;
	uint32_t heads;
	uint32_t sectors;
	uint64_t lba_count;
	uint16_t bytes_per_sector;
	uint16_t dpte_off;
	uint16_t dpte_seg;
} __attribute__((packed));

/* device parameter table extension */
struct dpte {
    uint16_t io_port;
    uint16_t control_port;
    uint8_t head_reg_upper;
    uint8_t bios_vendor_specific;
    uint8_t irq_info;
    uint8_t block_count_multiple;
    uint8_t dma_info;
    uint8_t pio_info;
    uint16_t flags;
    uint16_t reserved;
    uint8_t revision;
    uint8_t checksum;
} __attribute__((packed));

typedef struct disk_device {
	const char *name; /* device name */	
	enum DISK_DEVICE_TYPE id; /* device id */
} disk_device_t;

struct bios_disk_data {
	int drive;
	uint32_t cylinders;
	uint32_t heads;
	uint32_t sectors;
	uint32_t flags;
};

typedef struct disk {
	int id;
	disk_device_t dev; /* underlying device */
	uint64_t total_sectors;
	uint64_t first_sector;
	struct partition *partition;
	int max_partition;
	uint16_t log_sector_size; // logarithm (base2) of sector size
	struct bios_disk_data *data; 
} disk_t;

void disk_create_index(void);
disk_t *disk_get_by_drive(uint16_t drive);
disk_t *get_boot_disk(void);
struct partition *disk_get_partition_by_ids(uint16_t drive, uint16_t partition);

void disk_read(disk_t *disk, uint64_t loc, uint64_t size, void *buf);

#endif // !__DISK_H__
