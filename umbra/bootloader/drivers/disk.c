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

void 
disk_create_index(void) 
{
	/* reference: https://wiki.osdev.org/Memory_Map_(x86)
	 * BIOS Data Area (BDA) has # of hard disk drives detected at 0x0475 (byte) 	
	 * 0x0475 = 0x0040:0x0075
	 * */
	uint8_t bda_disk_count = mminb(rm_desegment(0x0040, 0x0075));

	/* why am I bothering to support optical drives? */
	uint32_t optical_indices = 1, hdd_indices = 1, consumed_bda_disks = 0;

	struct bios_drive_params *drp = (struct bios_drive_params *)SCRATCH_ADDR;
	struct int_regs regs;


	/* TODO: LOOP OVER ALL DRIVES */

	uint8_t drive = 0x80;

	memset(drp, 0, sizeof(*drp));

	regs.eax = 0x4800;
	regs.edx = drive;

	regs.ds = ((uint64_t)drp) >> 4;
	regs.esi = ((uint64_t)drp) & 0xf;
	drp->buf_size = sizeof(struct bios_drive_params);

	/* 
	 * INT 13h AH=48h: Extended Read Drive Parameters
	 * */
	rm_int(0x13, &regs);

	if (regs.flags & 1) {
		putstr("[PANIC] what\n", COLOR_RED, COLOR_BLK);
		while (1);
	}

	bool is_removable = drp->info_flags & (1 << 2);

	struct dpte *dpte = NULL;
	if (drp->buf_size >= 0x01fe // 510
			&& (drp->dpte_seg != 0x0000 || drp->dpte_off != 0x0000) // not zero
			&& (drp->dpte_seg != 0xffff || drp->dpte_off != 0xffff)) { // 16-bit, so data buffer can't cross a 64kB boundary without causing address wraparound
		dpte = (void *)rm_desegment(drp->dpte_seg, drp->dpte_off);	
		if ((dpte->control_port & 0xff00) != 0xa000) {
			// check for removable (5) or ATAPI (6)
			is_removable = is_removable || ((dpte->flags & (1 << 5)) || (dpte->flags & (1 << 6)));
		}
	}

	struct volume *block = ext_mem_alloc(sizeof(struct volume));
	block->drive = drive;
	block->partition = 0;
	block->first_sector = 0;
	block->sector_count = drp->lba_count;
	block->max_partition = -1;


	putstr("sector_count: ", COLOR_GRN, COLOR_BLK);
	{
		char res[16];
		itoa(block->sector_count, res, 10);
		putstr(res, COLOR_GRN, COLOR_BLK);
	}
	putstr("\n", COLOR_GRN, COLOR_BLK);
	/* sector_count seems correct to me based on bootloader size */

	/* TODO: read kernel - need GPT / FAT32 for this? Should write a bare-bones kernel to read into memory */
	/* I've already read the entirety of the bootloader (core.img) into memory, so how would this play out? 
	 * haven't kernel into memory (currently there is no kernel) */

}


