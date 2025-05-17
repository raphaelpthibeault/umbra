#include <types.h>
#include "vga.h"

void * 
memset(void * dest, int c, long n) 
{
	__asm__ volatile("cld; rep stosb"
	             : "=c"((int){0})
	             : "D"(dest), "a"(c), "c"(n)
	             : "flags", "memory");
	return dest;
}

extern char _bss_start[];
extern char _bss_end[];

void __attribute__((noreturn))
boot_main(void)
{
	clearwin(COLOR_WHT, COLOR_BLK);
	set_cursor_pos(0, 0);
	putstr("\n*** UMBRA BOOTLOADER STAGE2 ***\n", COLOR_GRN, COLOR_BLK); // weird bug where first line starts with a space

	memset(&_bss_start,0,(uintptr_t)&_bss_end-(uintptr_t)&_bss_start);
	putstr("Zeroed BSS\n", COLOR_GRN, COLOR_BLK);

	while (1);
}

