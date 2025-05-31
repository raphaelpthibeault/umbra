#include "partition.h"
#include <types.h>
#include <lib/misc.h>
#include <mm/pmm.h>
#include <drivers/vga.h>
#include <drivers/disk.h>

bool
is_valid_mbr(disk_t *disk) 
{
	uint8_t mbr[70];
	uint8_t hint8;
	uint16_t hint16;
	char hintc[64];

	disk_read(disk, 440, 70, &mbr);
	
	putstr("MBR:\n", COLOR_GRN, COLOR_BLK);
	putstr(mbr, COLOR_GRN, COLOR_BLK);
	putstr("\n", COLOR_GRN, COLOR_BLK);
	/* obviously this is loopable but since it's just 4 iterations I'll unroll it manually */

	hint8 = mbr[6]; // 446
	if (hint8 != 0x00 && hint8 != 0x80) {
		return false;
	}

	hint8 = mbr[22]; // 462
	if (hint8 != 0x00 && hint8 != 0x80) {
		return false;
	}

	hint8 = mbr[38]; // 478
	if (hint8 != 0x00 && hint8 != 0x80) {
		return false;
	}

	hint8 = mbr[54]; // 494
	if (hint8 != 0x00 && hint8 != 0x80) {
		return false;
	}
	
	/* same for the check if it's a file system boot record */
		
	disk_read(disk, 4, 8, &hintc);
	if (memcmp(&hintc, "_ECH_FS_", 8) == 0) {
		return false;
	}

	disk_read(disk, 3, 4, &hintc);
	if (memcmp(&hintc, "NTFS", 4) == 0) {
		return false;
	}

	disk_read(disk, 54, 3, &hintc);
	if (memcmp(&hintc, "FAT", 3) == 0) {
		return false;
	}

	disk_read(disk, 82, 3, &hintc);
	if (memcmp(&hintc, "FAT", 3) == 0) {
		return false;
	}

	disk_read(disk, 3, 5, &hintc);
	if (memcmp(&hintc, "FAT32", 5) == 0) {
		return false;
	}

	disk_read(disk, 1080, sizeof(uint16_t), &hint16);
	if (hint16 == 0xef53) { // ext2 or ext4
		return false;
	}

	return true;
}

struct mbr_entry {
	uint8_t attributes;
	uint8_t chs_first_sector[3];
	uint8_t type;
	uint8_t chs_last_sector[3];
	uint32_t first_sector;
	uint32_t total_sectors;
};

int 
partitions_get(disk_t *disk) 
{
	int partition;
	int count = 0;

	/* must be MBR */
	if (!is_valid_mbr(disk)) {
		return INVALID_TABLE;	
	}	

	struct partition *parts = NULL;

	for (partition = 0; partition < 4; ++partition) {
		struct mbr_entry entry;
		uint64_t entry_offset = 0x1be + (partition * sizeof(struct mbr_entry));
		disk_read(disk, entry_offset, sizeof(struct mbr_entry), &entry);

		if (entry.type != 0x0f && entry.type != 0x05) {
			continue;
		}

		parts = memmap_realloc(
				parts,
				partition * sizeof(struct partition),
				(partition + 1) * sizeof(struct partition)
		);

		parts[partition].number = partition;
		parts[partition].first_sector = entry.first_sector;
		parts[partition].total_sectors = entry.total_sectors;
		
		++count;
	}

	if (count == 0) {
		memmap_free(parts, sizeof(struct partition));
		parts = NULL;
	}

	putstr("Drive: 0x", COLOR_GRN, COLOR_BLK);
	{
		char res[8];
		itoa(disk->data->drive, res, 16);
		putstr(res, COLOR_GRN, COLOR_BLK);
	}
	putstr(" has ", COLOR_GRN, COLOR_BLK);
	{
		char res[8];
		itoa(count, res, 10);
		putstr(res, COLOR_GRN, COLOR_BLK);
	}
	putstr(" partitions\n", COLOR_GRN, COLOR_BLK);

	disk->max_partition = count;
	disk->partition = parts;
	
	return END_OF_TABLE;
}

