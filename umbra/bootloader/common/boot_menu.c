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
	terminal_write("***** Bootloader Terminal *****\n\n\n");
	terminal_write("Select a kernel to booot:\n\n");

	/* TODO: flush IRQs */
	/* TODO: init IO APICS */

	/* TODO: print fs tree */

	size_t selected_entry = 0;
	size_t max_entries = 3;
	char *entries[] = 
	{
		"option 0",
		"option 1",
		"option 2",
	};

	size_t x, y, top, bottom;
	terminal_get_cursor_pos(&x, &y);
	top = y;	
	bottom = y + max_entries - 1;
	serial_print("top: %d, bottom: %d\n", top, bottom); 

	/* newline sets cursor pos to next line (which then colors that pos), so avoid \n */
	for (size_t i = 0; i < max_entries; ++i)
	{
		terminal_set_cursor_pos(0, y + i);
		terminal_print("%s", entries[i]);
	}

	char c;
	for (;;) 
	{
		terminal_set_cursor_pos(0, top + selected_entry);
		terminal_set_color(0x00000000, 0x00aaaaaa);
		terminal_print("%s", entries[selected_entry]);
		c = getchar(); /* blocking call */
		switch (c)
		{
			case GETCHAR_CURSOR_UP:
				terminal_set_cursor_pos(0, top + selected_entry);
				terminal_set_color(0x00aaaaaa, 0x00000000);
				terminal_print("%s", entries[selected_entry]);
				if (selected_entry == 0)
					selected_entry = max_entries - 1;
				else
					--selected_entry;
				break;
			case GETCHAR_CURSOR_DOWN:
				terminal_set_cursor_pos(0, top + selected_entry);
				terminal_set_color(0x00aaaaaa, 0x00000000);
				terminal_print("%s", entries[selected_entry]);
				if (selected_entry == max_entries - 1)
					selected_entry = 0;
				else
					++selected_entry;
				break;
			case GETCHAR_ENTER:
				serial_print("Boot Menu: Selected '%s'\n", entries[selected_entry]);
				goto load_kernel;
		}
	}

	/* TODO: load user's chosen kernel (umbra) */
load_kernel:
	terminal_disable_cursor();
	terminal_set_color(0x00aaaaaa, 0x00000000);
	terminal_clear();

	while(1);	
}

