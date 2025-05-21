#include <types.h>
#include "vga.h"
#include "e820.h"
#include <mm/pmm.h>
#include <misc.h>

char* 
itoa(uint32_t value, char* result, uint8_t base) 
{
	// check that the base is valid
	if (base < 2 || base > 36) { 
		*result = '\0'; return result; 
	}

	char* ptr = result, *ptr1 = result, tmp_char;
	uint64_t tmp_value;


	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	/*
	// apply negative sign
	if (tmp_value < 0) {
		*ptr++ = '-';
	}*/

	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}

	return result;
}

void __attribute__((noreturn))
boot_main(uint8_t boot_drive)
{
	clearwin(COLOR_WHT, COLOR_BLK);
	set_cursor_pos(0, 0);
	putstr("\n*** UMBRA BOOTLOADER STAGE2 ***\n", COLOR_GRN, COLOR_BLK); // weird bug where first line starts with a space

	memset(&_bss_start,0,(uintptr_t)&_bss_end-(uintptr_t)&_bss_start);
	putstr("Started booting ...\n", COLOR_GRN, COLOR_BLK);

	do_e820();
	putstr("Got e820 memmap\n", COLOR_GRN, COLOR_BLK);

	memmap_init();
	putstr("Init'd bootloader memmap\n", COLOR_GRN, COLOR_BLK);
	{
		putstr("\t# entries: ", COLOR_GRN, COLOR_BLK);
		char res[8];
		itoa(memmap_entries, res, 10);
		putstr(res, COLOR_GRN, COLOR_BLK); 
		putstr("\n", COLOR_GRN, COLOR_BLK);
	}

	while (1);
}

