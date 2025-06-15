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

	if (!terminal_init())
	{
		serial_print("[PANIC] error initializating terminal\n");
		while (1);
	}

	terminal_write("*** Bootloader Terminal***\n>\n");

	/* print fs tree */

	/* keyboard interrupts */

	/* load user's chosen kernel (umbra) */

	while(1);	
}



