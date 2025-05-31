#include <drivers/disk.h>
#include <types.h>
#include <drivers/vga.h>
#include <lib/misc.h>
#include <mm/pmm.h>
#include <lib/partition.h>

#include <arch/x86_64/real.h>
#include <arch/x86_64/cpu.h>

disk_t *disk_list = NULL;
uint32_t disk_list_idx = 0;


struct dap {
	uint8_t length;
	uint8_t reserved;
	uint16_t blocks;
	uint32_t buffer;
	uint64_t block;
} __attribute__((packed));

disk_t *
disk_get_by_drive(uint16_t drive) {
	for (uint32_t i = 0; i < disk_list_idx; ++i) {
		if (disk_list[i].data->drive == drive) {
			return &disk_list[i];
		}
	}	
	return NULL;
}

static void
test_disk_read(disk_t *disk, uint64_t loc, uint64_t size, void *buf) 
{
	/* memory layout
	 *            [sectors to read][dap]
	 * SCRATCH_ADDR
	 *
	 * SCRATCH_ADDR = 0x68000
	 * SCRATCH_SEG = 0x6800
	 * */
	
	uint64_t bottom = ALIGN_DOWN(loc, 512);
	uint64_t top = ALIGN_UP(loc+size, 512);
	uint64_t total_bytes = (top - bottom);
	uint64_t sectors_to_read = (total_bytes) / 512;

	struct dap *dap = (struct dap *)(SCRATCH_ADDR + total_bytes);
	dap->length = sizeof(struct dap); /* size */
	dap->reserved = 0; /* reserved byte */
	dap->blocks = sectors_to_read; /* number of sectors */
	dap->buffer = SCRATCH_SEG << 0x10; /* address of buffer, recall SEG:ADDR */
	dap->block = loc >> disk->log_sector_size; /* absolute (start) sector to read */

	struct int_regs regs = {0};

	regs.eax = 0x4200;
	regs.ds = ((uintptr_t)dap & 0xffff0000) >> 4;
	regs.esi = ((uintptr_t)dap & 0xffff);

	regs.edx = disk->data->drive;
	regs.flags = 0x200;

	rm_int(0x13, &regs);

	if ((regs.eax >> 8) & 0xff) {
		putstr("[PANIC] flags\n", COLOR_RED, COLOR_BLK);
		while (1);
	}

	memcpy(buf, (void *)SCRATCH_ADDR, size);
}

void 
disk_create_index(void) 
{
	/* reference: https://wiki.osdev.org/Memory_Map_(x86)
	 * BIOS Data Area (BDA) has # of hard disk drives detected at 0x0475 (byte) 	
	 * 0x0475 = 0x0040:0x0075
	 * */
	uint8_t bda_hdd_count = mminb(rm_desegment(0x0040, 0x0075));
	uint8_t consumed_bda_hdds = 0;

	if (bda_hdd_count == 0) {
		putstr("[PANIC] what\n", COLOR_RED, COLOR_BLK);
		while (1);
	}

	/* there can be several physical mediums: CDs, optical disks, floppies, etc
	 * I'll support just bios disk for now */
	disk_device_t bios_disk_dev = {
		.name = "biosdisk",
		.id = BIOS_DISK,
	};

	for (uint16_t drive = 0x80; drive < 0x100; ++drive) {
// can't have a loop with multiple counter variables, what is going on?
// surely it's because I'm cloberring a register here somewhere
// error: unsupported size for integer register 
		struct int_regs regs = {0};

		struct bios_drive_params *drp = (struct bios_drive_params *)SCRATCH_ADDR;
		memset(drp, 0, sizeof(*drp));

		disk_t *disk = ext_mem_alloc(sizeof(struct disk));
		disk->id = drive;
		disk->dev = bios_disk_dev;
		disk->log_sector_size = 9; // HDD log_2(512) = 9

		struct bios_disk_data *data = ext_mem_alloc(sizeof(struct bios_disk_data));
		data->drive = drive;
		data->flags = FLAG_LBA;

		/* setup extended read */	
		regs.eax = 0x4800;
		regs.edx = drive;

		regs.ds = ((uintptr_t)drp) >> 4;
		regs.esi = ((uintptr_t)drp) & 0xf;
		drp->buf_size = sizeof(struct bios_drive_params);

		/* 
		* INT 13h AH=48h: Extended Read Drive Parameters
		* */
		rm_int(0x13, &regs);

		if (regs.flags & 1) {
			putstr("[PANIC] what\n", COLOR_RED, COLOR_BLK);
			while (1);
		}

		disk->total_sectors = drp->lba_count;
		disk->partition = NULL;
		disk->max_partition = -1;
		disk->first_sector = 0;

		data->cylinders = drp->cylinders;
		data->heads = drp->heads;
		data->sectors = drp->sectors;

		disk->data = data;
		
		/* register disk */
		struct disk *dp = NULL;
		{
			disk_list = memmap_realloc(
					disk_list, 
					disk_list_idx * sizeof(struct disk),
					(disk_list_idx + 1) * sizeof(struct disk)
			); 

			/* get disk on stack */
			struct disk d;
			d = *disk;
			memmap_free(disk, sizeof(struct disk));
			disk = NULL;

			disk_list[disk_list_idx] = d;
			dp = &disk_list[disk_list_idx];
			++disk_list_idx;
		}
		if (dp == NULL) {
			putstr("[PANIC] disk pointer shenanigans error\n", COLOR_RED, COLOR_BLK);
			while (1);
		}
		
		uint8_t first_sector[512];
		test_disk_read(dp, 0, 512, &first_sector);
		
		// little endian
		uint16_t magic = (first_sector[511] << 8) + first_sector[510];
		putstr("bios magic: 0x", COLOR_GRN, COLOR_BLK);
		{
			char res[16];
			itoa(magic, res, 16);
			putstr(res, COLOR_GRN, COLOR_BLK);
		}
		putstr("\n", COLOR_GRN, COLOR_BLK);

		/*
		if(partitions_get(dp) != END_OF_TABLE) {
			putstr("[PANIC] partitions_get()\n", COLOR_RED, COLOR_BLK);
			while (1);
		}*/

		++consumed_bda_hdds;
		if (consumed_bda_hdds >= bda_hdd_count) {
			break;
		}
	}
}


/* return number of sectors that can be safely read at a time */
static size_t
get_safe_sectors(disk_t *disk, uint64_t sector)
{
	// TODO: math 	
	(void)disk;
	(void)sector;
	return 127;
}

#define READ 0
#define WRITE 1

static int
disk_rw_int13(int ah, int drive, struct dap *dap)
{
	struct int_regs regs = {0};

	regs.eax = ah << 8;
	regs.ds = ((uintptr_t)dap & 0xffff0000) >> 4;
	regs.esi = ((uintptr_t)dap & 0xffff);

	regs.edx = drive;
	regs.flags = 0x200;

	rm_int(0x13, &regs);

	return (regs.eax >> 8) & 0xff;
}

static void
disk_rw(int cmd, disk_t *disk, uint64_t sector, size_t size, unsigned segment)
{
	struct bios_disk_data *data = disk->data;

	struct dap *dap = (struct dap *)(SCRATCH_ADDR + (data->sectors << disk->log_sector_size));
	dap->length = sizeof(*dap);
	dap->reserved = 0;
	dap->blocks = size;
	dap->buffer = segment << 16; /* recall format SEG:ADDR */
	dap->block = sector;
	
	if (disk_rw_int13(cmd + 0x42, data->drive, dap)) {
		putstr("[PANIC] Couldn't read\n", COLOR_RED, COLOR_BLK);
		while (1);
	}
}

void 
disk_read(disk_t *disk, uint64_t start /* bytes */, size_t size /* bytes */, void *buf)
{
	/* let's say we read and cache 16 sectors at a time (completely arbitrary) */
	uint64_t read_length = 16;
	uint64_t cache_size = read_length * 512;
	uint8_t *cache = ext_mem_alloc(cache_size);
	memset(cache, 0, cache_size);

	uint64_t read_bytes = 0;

	uint64_t bottom = ALIGN_DOWN(start, 512);
	uint64_t top = ALIGN_UP(start+size, 512);
	uint64_t total_bytes = (top - bottom);

	while (read_bytes < size) {
		// should probably memset cache but I think we're fine ?
		uint64_t sectors = (total_bytes - read_bytes) / 512;
		uint64_t sector = (bottom + read_bytes) / 512;

		if (sectors > read_length) {
			sectors = read_length;			
		}

		disk_read_sectors(disk, sector, sectors, cache);
	
		uint64_t offset = (start + read_bytes) % cache_size;

		uint64_t chunk = size - read_bytes;
		if (chunk > cache_size - offset) {
			chunk = cache_size - offset;
		}

		memcpy(buf + read_bytes, &cache[offset], chunk);
		read_bytes += chunk;
	}

	memmap_free(cache, cache_size);	
}

void
disk_read_sectors(disk_t *disk, uint64_t sector, size_t sectors, void *buf)
{
	while (sectors) {
		size_t len;

		len = get_safe_sectors(disk, sector);
		if (len > sectors) {
			len = sectors;
		}
		
		disk_rw(READ, disk, sector, len, SCRATCH_SEG);
		memcpy(buf, (void *)SCRATCH_ADDR, len << disk->log_sector_size);

		buf += len << disk->log_sector_size;
		sector += len;
		sectors -= len;
	}
}

