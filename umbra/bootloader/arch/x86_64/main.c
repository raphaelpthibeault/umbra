#include <types.h>
#include <drivers/vga.h>
#include "e820.h"
#include <mm/pmm.h>
#include <lib/misc.h>
#include "idt.h"
#include <drivers/disk.h>

noreturn void
boot_main(uint8_t boot_drive)
{
	disk_t *boot_disk;

	clearwin(COLOR_WHT, COLOR_BLK);
	set_cursor_pos(0, 0);
	putstr("\n*** UMBRA BOOTLOADER STAGE2 ***\n", COLOR_GRN, COLOR_BLK); // weird bug where first line starts with a space

	memset(&_bss_start,0,(uintptr_t)&_bss_end-(uintptr_t)&_bss_start);
	putstr("Started booting ...\n", COLOR_GRN, COLOR_BLK);

	do_e820();
	memmap_init();
	set_idt();
	disk_create_index();
	putstr("Created disk index\n", COLOR_GRN, COLOR_BLK);


	
	boot_disk = disk_get_by_drive(boot_drive);
	if (boot_disk == NULL) {
		putstr("[Panic] Could not get boot disk!\n", COLOR_RED, COLOR_BLK);
		while (1);
	}

	putstr("Got boot disk...\n", COLOR_GRN, COLOR_BLK);
	putstr("\tDevice name: ", COLOR_GRN, COLOR_BLK);
	putstr(boot_disk->dev.name, COLOR_GRN, COLOR_BLK);
	putstr("\n\tSector count: 0x", COLOR_GRN, COLOR_BLK);
	{
		char res[32];
		itoa(boot_disk->total_sectors, res, 16);
		putstr(res, COLOR_GRN, COLOR_BLK);
	}
	putstr("\n\tTotal Memory: 0x", COLOR_GRN, COLOR_BLK);
	{
		char res[32];
		itoa(boot_disk->total_sectors * 512, res, 16);
		putstr(res, COLOR_GRN, COLOR_BLK);
	}
	putstr("\n", COLOR_GRN, COLOR_BLK);

	// jump to boot_menu

	while (1);
}

