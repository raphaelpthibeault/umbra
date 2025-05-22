#include <types.h>
#include <drivers/vga.h>
#include "e820.h"
#include <mm/pmm.h>
#include <lib/misc.h>
#include "idt.h"

void __attribute__((noreturn))
boot_main(uint8_t boot_drive)
{
	(void)boot_drive;

	clearwin(COLOR_WHT, COLOR_BLK);
	set_cursor_pos(0, 0);
	putstr("\n*** UMBRA BOOTLOADER STAGE2 ***\n", COLOR_GRN, COLOR_BLK); // weird bug where first line starts with a space

	memset(&_bss_start,0,(uintptr_t)&_bss_end-(uintptr_t)&_bss_start);
	putstr("Started booting ...\n", COLOR_GRN, COLOR_BLK);

	do_e820();
	putstr("Got e820 memmap\n", COLOR_GRN, COLOR_BLK);

	memmap_init();
	putstr("Init'd bootloader memmap\n", COLOR_GRN, COLOR_BLK);

	set_idt();
	putstr("Set bootloader IDT\n", COLOR_GRN, COLOR_BLK);

	while (1);
}

