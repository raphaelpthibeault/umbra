#include <mm/pmm.h>
#include <types.h>

/* TODO: ifdef BIOS and UEFI, and arch as well */
#include <arch/x86_64/e820.h>
#include <arch/x86_64/vga.h>
#include <arch/x86_64/acpi.h>

#define FREE_MEM 0x100000 /* RAM, extended memory ; 00x100000-x00efffff  */
#define memmap_max_entries ((size_t)512)

struct memmap_entry memmap[memmap_max_entries];
size_t memmap_entries = 0;

/* reference:
 * https://wiki.osdev.org/Memory_Map_(x86)
 **/
void
init_memmap(void)
{
	for (size_t i = 0; i < e820_entries; ++i) {
		if (memmap_entries == memmap_max_entries) {
			/* exhausted memory map */
			putstr("[Panic] memmap exhausted", COLOR_RED, COLOR_BLK);
			while(1);
		}
		
		memmap[memmap_entries] = e820_map[i];

		uint64_t top = memmap[memmap_entries].base + memmap[memmap_entries].length;
		if (memmap[memmap_entries].type == MEMMAP_USABLE) {
			if (memmap[memmap_entries].base >= EBDA && memmap[memmap_entries].base < FREE_MEM) {
				if (top <= FREE_MEM) {
					continue;
				}
				/* let length be the remainder, and move it to the free memory */
				memmap[memmap_entries].length -= (FREE_MEM - memmap[memmap_entries].base);
				memmap[memmap_entries].base = FREE_MEM;
			}
			
			if (top > EBDA && top <= FREE_MEM) {
				// but base is fine
				memmap[memmap_entries].length -= top - EBDA;
			}
		}

		++memmap_entries;
	}

}
