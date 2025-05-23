#include "e820.h"
#include "real.h"
#include <mm/pmm.h>
#include <drivers/vga.h>
#include <lib/misc.h>

#define MAX_E820_ENTRIES 256

struct memmap_entry e820_map[MAX_E820_ENTRIES];
size_t e820_entries;

struct smap_entry {
	uint32_t basel;
	uint32_t baseh;
	uint32_t lengthl;
	uint32_t lengthh;
	uint32_t type;
	uint32_t acpi;
} __attribute__((packed));

void 
do_e820(void)
{
	struct smap_entry *buf = (struct smap_entry *)SCRATCH_ADDR;

	struct int_regs regs;
	regs.ebx = 0;

	for (size_t i = 0; i < MAX_E820_ENTRIES; ++i) {
		memset(buf, 0, sizeof(*buf));

		regs.flags = 0x200;
		regs.es = ((uint64_t)buf) >> 4;
		regs.edi = ((uint64_t)buf) & 0xf;
		regs.ecx = 24; // sizeof (smap,memmap)_entry
		regs.edx = 0x534d4150;
		regs.eax = 0xe820;

		rm_int(0x15, &regs);

		if (regs.ebx == 0 
				|| (regs.flags & 0x1)
				|| (regs.eax != 0x534d4150)) {
			return;
		}
	
		struct memmap_entry entry;
		entry.base = ((uint64_t)buf->baseh << 32) | buf->basel;
		entry.length = ((uint64_t)buf->lengthh << 32) | buf->lengthl;
		entry.type = buf->type;
		entry.acpi = buf->acpi;

		e820_map[i] = entry;
		++e820_entries;
	}
}

