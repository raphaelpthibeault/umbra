#include <drivers/disk.h>
#include <types.h>
#include <drivers/vga.h>
#include <lib/misc.h>
#include <mm/pmm.h>
#include <lib/partition.h>

#include <arch/x86_64/real.h>
#include <arch/x86_64/cpu.h>

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


disk_t *disk_list = NULL;
uint32_t disk_list_idx = 0;

disk_t *
disk_get_by_drive(uint16_t drive) {
	for (uint32_t i = 0; i < disk_list_idx; ++i) {
		if (disk_list[i].drive == drive) {
			return &disk_list[i];
		}
	}	
	return NULL;
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

		disk_t *disk = ext_mem_alloc(sizeof(struct disk));
		disk->dev = bios_disk_dev;
		struct bios_drive_params *drp = (struct bios_drive_params *)SCRATCH_ADDR;
		memset(drp, 0, sizeof(*drp));

		disk->drive = drive;

		struct int_regs regs = {0};

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
		
		/* register disk */
		disk_list = memmap_realloc(
				disk_list, 
				disk_list_idx * sizeof(struct disk),
				(disk_list_idx + 1) * sizeof(struct disk)
		); 

		// get the value of d onto the stack
		struct disk d;
		d = *disk;
		memmap_free(disk, sizeof(struct disk));
		disk = NULL;

		disk_list[disk_list_idx++] = d;

		// TODO: partitions 

		for (int part = 0; ; ++part) { // upper limit on partitions???
			// loc: disk->first_sector
			// count  1? the first
			// iterate...
			break;
		}
	

		++consumed_bda_hdds;
		if (consumed_bda_hdds >= bda_hdd_count) {
			break;
		}
	}
}

struct dap {
	uint16_t size;
	uint16_t count;
	uint16_t segment;
	uint16_t offset;
	uint64_t total_sectors;
};

void
disk_read(disk_t disk, void *buf, uint64_t loc, uint64_t count)
{

	
}

