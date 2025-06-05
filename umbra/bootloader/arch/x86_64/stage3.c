#include <types.h>
#include <common/boot_menu.h>
#include <drivers/vga.h>

noreturn void
_stage3_start(void)
{
	clearwin(COLOR_GRN, COLOR_BLK);
	boot_menu();		
}

