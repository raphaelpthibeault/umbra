#include <types.h>
#include "vga.h"


void __attribute__((noreturn))
boot_main(void)
{
	clearwin(COLOR_WHT, COLOR_BLK);
	set_cursor_pos(0, 0);
	putstr("\n*** UMBRA BOOTLOADER STAGE2 ***\n", COLOR_GRN, COLOR_BLK); // weird bug where first line starts with a space


	while (1);
}

