#include <types.h>
#include <drivers/vga.h>
#include "e820.h"
#include <mm/pmm.h>
#include <lib/misc.h>
#include "idt.h"
#include <drivers/disk.h>
#include <fs/file.h>

extern char _stage3_addr[];

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

	boot_disk = disk_get_by_drive(boot_drive);
	if (boot_disk == NULL) {
		putstr("[Panic] Could not get boot disk!\n", COLOR_RED, COLOR_BLK);
		while (1);
	}

	/* find stage3 aka the boot menu */
	putstr("Finding stage3...\n", COLOR_GRN, COLOR_BLK);
	/* stage3 is the boot_menu where user chooses kernel to load */
	bool found_menu = false;
	struct filehandle *stage3;
	for (int i = 0; i < boot_disk->max_partition; ++i) {
		struct partition part = boot_disk->partition[i];

		if ((stage3 = fopen(&part, "/boot/bootloader/stage3.sys")) == NULL) {
			continue;
		}
		found_menu = true;
		/* definitions in stage3.ld; read stage3 into _stage3_addr */
		fread(stage3, // fh
				_stage3_addr, // buf
				(uintptr_t)_stage3_addr - 0xf000, // loc = addr - addr_start ; should be 0 
				stage3->size - ((uintptr_t)_stage3_addr - 0xf000)); // count = size - loc ; should result in size
		/* close */
		fclose(stage3);
	}
	if (!found_menu) {
		putstr("[PANIC] could not find stage3.sys!\n", COLOR_RED, COLOR_BLK);
		while (1);
	}

	/* jump to boot_menu */
	putstr("Found stage3. Jumping to stage3...\n", COLOR_GRN, COLOR_BLK);

	typedef void (*stage3_entry_func_t)(void);
	stage3_entry_func_t stage3_start = (stage3_entry_func_t)((uintptr_t)_stage3_addr);
	stage3_start();

	__builtin_unreachable();
}

