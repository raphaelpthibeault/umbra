#include "boot_menu.h"
#include <types.h>
#include <drivers/vga.h>
#include <common/terminal.h>
#include <drivers/serial.h>

#include <arch/x86_64/kbd.h>


#include <drivers/vbe.h>
#include <lib/framebuffer.h>
noreturn void
_boot_menu(void)
{
	/* VGA turn off */
	clearwin(COLOR_WHT, COLOR_BLK);
	hide_cursor();

	if (!terminal_init())
	{
		serial_print("[PANIC] error initializating terminal\n");
		while (1);
	}

	serial_print("Terminal: Initialized\n");
	terminal_write("***** Bootloader Terminal *****\n>\n");

	/* TODO: flush IRQs */
	/* TODO: init IO APICS */

	/* TODO: print fs tree */

	int c;
	for (;;) {
		c = getchar();
		serial_print("kbd in c: %d \n", c);
	}

	/* TODO: load user's chosen kernel (umbra) */

	while(1);	
}



