#include "boot_menu.h"
#include <types.h>
#include <drivers/vga.h>

noreturn void
_boot_menu(void)
{
	clearwin(COLOR_GRN, COLOR_BLK);
	putstr("UMBRA BOOT MENU\n", COLOR_GRN, COLOR_BLK);
	/* VGA turn off */


	while(1);	
}



