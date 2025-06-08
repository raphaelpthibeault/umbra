#include "boot_menu.h"
#include <types.h>
#include <drivers/vga.h>
#include <common/terminal.h>
#include <drivers/serial.h>

noreturn void
_boot_menu(void)
{
	/* VGA turn off */
	clearwin(COLOR_GRN, COLOR_BLK);
	hide_cursor();

	terminal_init();

	while(1);	
}



