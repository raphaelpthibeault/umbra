#include "boot_menu.h"
#include <types.h>
#include <drivers/vga.h>
#include <drivers/serial.h>
#include <drivers/disk.h>
#include <common/terminal.h>
#include <common/config.h>
#include <arch/x86_64/kbd.h>

noreturn void
_boot_menu(uint8_t drive)
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
	terminal_print("***** Bootloader Terminal *****\n\n\n");
	terminal_print("Select a kernel to booot:\n\n");

	/* TODO: flush IRQs */
	/* TODO: init IO APICS */

	disk_t *boot_disk;
	boot_disk = disk_get_by_drive(drive);
	if (boot_disk == NULL) {
		serial_print("[PANIC] Could not get boot disk!\n");
		while (1);
	}

	if (config_init_disk(boot_disk) < 0)
	{
		serial_print("[PANIC] Could not init config\n");
		while (1);
	}
	serial_print("Config: Initialized from disk\n");

	if (menu_tree == NULL)
	{
		serial_print("[PANIC] the fuck? menu tree is null\n");
		while (1);
	}

	/* TODO: print menu tree */

	size_t selected_entry = 0;
	size_t max_entries = 3;
	char *entries[] = 
	{
		"option 0",
		"option 1",
		"option 2",
	};

	size_t x, y, top;
	terminal_get_cursor_pos(&x, &y);
	top = y;	

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

