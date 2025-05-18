#include <types.h>
#include "vga.h"

void * 
memset(void *dest, int c, long n) 
{
	__asm__ volatile("cld; rep stosb"
	             : "=c"((int){0})
	             : "D"(dest), "a"(c), "c"(n)
	             : "flags", "memory");
	return dest;
}

char* 
itoa(int value, char* result, int base) 
{
	// check that the base is valid
	if (base < 2 || base > 36) { 
		*result = '\0'; return result; 
	}

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// apply negative sign
	if (tmp_value < 0) {
		*ptr++ = '-';
	}

	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}

	return result;
}

extern char _bss_start[];
extern char _bss_end[];

void __attribute__((noreturn))
boot_main(uint8_t boot_drive)
{
	clearwin(COLOR_WHT, COLOR_BLK);
	set_cursor_pos(0, 0);
	putstr("\n*** UMBRA BOOTLOADER STAGE2 ***\n", COLOR_GRN, COLOR_BLK); // weird bug where first line starts with a space

	memset(&_bss_start,0,(uintptr_t)&_bss_end-(uintptr_t)&_bss_start);
	putstr("Zeroed BSS\n", COLOR_GRN, COLOR_BLK);

	if (boot_drive != 0x80) {
		putstr("[Warning] BIOS boot drive not 'C:'!\n\t boot drive: ", COLOR_GRN, COLOR_BLK); 
		
		char res[1];
		itoa(boot_drive, res, 10);
		putstr(res, COLOR_GRN, COLOR_BLK); 
	}


	while (1);
}

