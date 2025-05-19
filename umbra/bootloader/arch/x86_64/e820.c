#include "e820.h"
#include "real.h"
#include <mm/pmm.h>
#include "vga.h"

#define MAX_E820_ENTRIES 256

struct memmap_entry e820_map[MAX_E820_ENTRIES];
size_t e820_entries;

void 
do_e820(void)
{
	struct int_regs regs;

	for (size_t i = 0; i < MAX_E820_ENTRIES; ++i) {

		struct memmap_entry entry;
		
		regs.eax = 0xe820;
		regs.ecx = 24;
		regs.edx = 0x534d4150;
		regs.edi = (uint32_t)&entry;
		regs.es = 0;

		rm_int(0x15, &regs);
		
		if (regs.flags & 0x1) {
			putstr("E820 Error, CF set!\n", COLOR_RED, COLOR_BLK);
			e820_entries = i; 
			return;
		}

		/* if ebx resets to 0, list is complete */
		if (regs.ebx == 0) {
			e820_entries = i;
			return;
		}

		e820_map[i] = entry;

		/* ebx = 0 implies list is only 1 entry long (worthless) */
		if (!regs.ebx) {
			e820_entries = ++i;
			return;
		}
	}
	
	/* panic */
	putstr("[Panic] \n", COLOR_RED, COLOR_BLK); 
	
}

