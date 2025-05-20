#include <mm/pmm.h>
#include <types.h>

/* TODO: ifdef BIOS and UEFI, and arch as well */
#include <arch/x86_64/e820.h>
#include <arch/x86_64/vga.h>
#include <arch/x86_64/acpi.h>

#define FREE_MEM 0x100000 /* RAM, extended memory ; 00x100000-x00efffff  */
#define memmap_max_entries ((size_t)512)
#define PAGE_SIZE 0x1000

struct memmap_entry memmap[memmap_max_entries];
size_t memmap_entries = 0;

#define DIV_ROUNDUP(a, b) ({ \
    __auto_type DIV_ROUNDUP_a = (a); \
    __auto_type DIV_ROUNDUP_b = (b); \
    (DIV_ROUNDUP_a + (DIV_ROUNDUP_b - 1)) / DIV_ROUNDUP_b; \
})

#define ALIGN_UP(x, a) ({ \
    __auto_type ALIGN_UP_value = (x); \
    __auto_type ALIGN_UP_align = (a); \
    ALIGN_UP_value = DIV_ROUNDUP(ALIGN_UP_value, ALIGN_UP_align) * ALIGN_UP_align; \
    ALIGN_UP_value; \
})

#define ALIGN_DOWN(x, a) ({ \
    __auto_type ALIGN_DOWN_value = (x); \
    __auto_type ALIGN_DOWN_align = (a); \
    ALIGN_DOWN_value = (ALIGN_DOWN_value / ALIGN_DOWN_align) * ALIGN_DOWN_align; \
    ALIGN_DOWN_value; \
})


static bool
page_align_entry(uint64_t *base, uint64_t *length) {
	if (*length < PAGE_SIZE) {
		return false;
	}

	uint64_t original_base = *base;

	*base = ALIGN_UP(*base, PAGE_SIZE);
	*length -= (*base - original_base);
	*length = ALIGN_DOWN(*length, PAGE_SIZE);

	if (!length) {
		return false;
	}

	return true;
}


/* reference:
 * https://wiki.osdev.org/Memory_Map_(x86)
 **/
void
memmap_init(void)
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

	memmap_sanitize_entries(memmap, &memmap_entries, false);

}

/* from what I can tell, for UEFI (and maybe risc/aarch) this should be true */
bool pmm_sanitizer_keep_first_page = false;

void 
memmap_sanitize_entries(struct memmap_entry *map, size_t *_count, bool page_align)
{
	size_t i, j;
	size_t count = *_count;	
	
	for (i = 0; i < count; ++i) {
		if (map[i].type != MEMMAP_USABLE) {
			continue;
		}

		/* check if the current entry has an overlap with other entries */
		for (j = 0; j < count; ++j) {
			if (j == i) {
				continue;
			}

			uint64_t base = map[i].base;
			uint64_t length = map[i].length;
			uint64_t top = base + length;

			uint64_t j_base = map[j].base;
			uint64_t j_length = map[j].length;
			uint64_t j_top = j_base + j_length;

			/* the i contains j case */	
			if ((j_base >= base && j_base < top) 
				&& (j_top >= base && j_top < top)) {
				/* TODO: surely there's some algorithm to to split the overlapping parts */
				putstr("[Panic] memmap entry fully contains another", COLOR_RED, COLOR_BLK);
				while(1);
			}

			/* j_base is in i
			 * [    i     ]
			 *          [   j   ] 
			 **/
			if (j_base >= base && j_base < top) {
				top = j_base;		
			}

			/* j_top is in i
			 *		     [    i     ]
			 * [    j    ] 
			 **/
			if (j_top >= base && j_top < top) {
				base = j_top;	
			}

			/* update, idempotent otherwise */
			map[i].base = base;
			map[i].length = top - base;
		}

		if (!map[i].length 
			|| (page_align && !page_align_entry(&map[i].base, &map[i].length))) {
			// remove i from memmap
			if (i < count) {
				map[i] = map[count - 1];
			}
			--count;
			--i;
		} 
	}

	/* remove length=0 entries and usable entries in the first page */
	for (i = 0; i < count; ++i) {
		if (map[i].type != MEMMAP_USABLE) {
			continue;
		}

		if (!pmm_sanitizer_keep_first_page && map[i].base < PAGE_SIZE) {
			if (map[i].base + map[i].length <= PAGE_SIZE) {
				goto del_memmap_entry;
			}

			// bigger than page size, adjust upwards 
			map[i].length -= (PAGE_SIZE - map[i].base);
			map[i].base = PAGE_SIZE;
		}

		if (map[i].length == 0) {
del_memmap_entry:	
			if (i < count - 1) {
				map[i] = map[count - 1];
			}
			--count;
			--i;
		}
	}

	/* sort the entries, then merge contiguous entries of the same type (bootloader-reclaimable and usable) */
	// insertion sort because we know the entries are already sorted by e820, or are almost sorted
	struct memmap_entry t;
	for (i = 0; i < count; ++i) {
		t = map[i];	
		for (j = i; j > 0 && map[j-1].base > t.base; --j) {
			map[j] = map[j-1];
		}
		map[j] = t;
	}
	// merge
	for (i = 0; i < count; ++i) {
		if (map[i].type != MEMMAP_BOOTLOADER_RECLAIMABLE 
			&& map[i].type != MEMMAP_USABLE) {
			continue;
		}
		
		if (map[i].type == map[i+1].type
			&& map[i].base + map[i].length == map[i+1].base) {
			map[i].length += map[i+1].length;

			for (j = i + 2; j < count; ++j) {
				map[j-1] = map[j];
			}
			--count;
			--i;
		}
	}

	*_count = count;
}

