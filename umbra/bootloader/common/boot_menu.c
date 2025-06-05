#include "boot_menu.h"
#include <types.h>
#include <drivers/vga.h>

noreturn void
_boot_menu(void)
{
	putstr("UMBRA BOOT MENU\n", COLOR_GRN, COLOR_BLK);

	while(1);	
}



