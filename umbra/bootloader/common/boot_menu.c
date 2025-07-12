#include "boot_menu.h"
#include <types.h>
#include <drivers/vga.h>
#include <drivers/serial.h>
#include <drivers/disk.h>
#include <lib/misc.h>
#include <mm/pmm.h>
#include <common/terminal.h>
#include <common/config.h>
#include <common/boot.h>
#include <arch/x86_64/kbd.h>

static size_t
print_tree(struct menu_entry *entry, size_t offset, size_t window, size_t level, size_t base_index, size_t selected_entry, struct menu_entry **selected_menu_entry)
{	
	size_t max_entries = 0;

	for (;;)
	{
		size_t curr_len = 0;
		if (entry == NULL)
		{
			break;
		}

		if ((base_index + max_entries < offset) || (base_index + max_entries >= offset + window))
		{
			goto skip_line;
		}

		if (level)
		{
			for (size_t i = level - 1; i > 0; --i)
			{
				struct menu_entry *parent = entry;
				for (size_t j = 0; j < i; ++j)
					parent = parent->parent;

				if (parent->next == NULL)
					terminal_print("  ");
				else
					terminal_print(" │");

				curr_len += 2;
			}
		}

		if (entry->sub)
		{
			terminal_print(entry->expanded ? "[-]" : "[+]");
		}
		else if (level)
		{
			terminal_print("──►");	
		}
		else
		{
			terminal_print("   ");
		}
		curr_len += 3;


		if (base_index + max_entries == selected_entry)
		{
			*selected_menu_entry = entry;		
			terminal_set_color(0x00000000, 0x00aaaaaa);
			terminal_print(" %s \n", entry->name);
			terminal_set_color(0x00aaaaaa, 0x00000000);
		} 
		else
		{
			terminal_print(" %s \n", entry->name);
		}

		curr_len += 1 + strlen(entry->name) + 1;

skip_line:
		if (entry->sub && entry->expanded)
		{
			max_entries += print_tree(entry->sub, offset, window, level + 1, base_index + max_entries + 1, selected_entry, selected_menu_entry);
		}
		++max_entries;
		entry = entry->next;
	}

	return max_entries;
}

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
	terminal_disable_cursor(); /* before printing anything to screen */

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

	size_t x, top;
	terminal_get_cursor_pos(&x, &top);

	size_t selected_entry = 0;
	struct menu_entry *selected_menu_entry = NULL;

	size_t max_entries = print_tree(menu_tree, 0, term_ctx->rows - 8, 0, 0, selected_entry, &selected_menu_entry);
	if (max_entries == 0 || selected_menu_entry == NULL)
	{
		serial_print("[PANIC] Boot Menu: Invalid configuration, no valid menu entries!\n");
		while (1);
	}

	char c;
	for (;;)
	{
		// fuck it reprint the entire thing, no cleverness about it
		terminal_set_cursor_pos(0, top);
	  print_tree(menu_tree, 0, term_ctx->rows - 8, 0, 0, selected_entry, &selected_menu_entry);

		c = getchar(); /* blocking call */
		switch (c)
		{
			case GETCHAR_CURSOR_UP:
				if (selected_entry == 0)
				{
					selected_entry = max_entries - 1;
				}
				else
				{
					--selected_entry;
				}
				break;
			case GETCHAR_CURSOR_DOWN:
				if (selected_entry == max_entries - 1)
				{
					selected_entry = 0;
				}
				else
				{
					++selected_entry;
				}
				break;
			case GETCHAR_ENTER:
				if (selected_menu_entry->sub != NULL)
				{
					selected_menu_entry->expanded = !selected_menu_entry->expanded;
					break;
				}
				serial_print("Boot Menu: Selected '%s'\n", selected_menu_entry->name);
				goto load_kernel;
		}
	}

load_kernel:
	terminal_set_color(0x00aaaaaa, 0x00000000);
	terminal_clear();

	terminal_set_cursor_pos(0, 0);

	boot(boot_disk, selected_menu_entry->body);
	__builtin_unreachable();
}

