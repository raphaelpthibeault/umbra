#include <types.h>
#include <multiboot2.h>
#include <protocol.h>
#include <drivers/serial.h>

#include <arch/x86_64/idt.h>
#include <arch/x86_64/memory_references.h>
#include <arch/x86_64/paging.h>

/* 
 * main routine for x86_64
 * */

struct memory_map *memory_map;

static uint64_t 
mb2_mmap_type_convert(uint64_t mb_type)
{
	switch (mb_type)
	{
		case MULTIBOOT_MEMORY_AVAILABLE:
			return MEMORY_MAP_FREE;
		case MULTIBOOT_MEMORY_RESERVED:
			return MEMORY_MAP_BUSY;
		case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
			return MEMORY_MAP_BUSY;
		case MULTIBOOT_MEMORY_NVS:
			return MEMORY_MAP_NOUSE;
		case MULTIBOOT_MEMORY_BADRAM:
			return MEMORY_MAP_NOUSE;
		default:
			return MEMORY_MAP_NOUSE;
	}
}

void *
mb2_find_tag(void *from, uint32_t type)
{
	serial_print("from: 0x%x\n", V2P(from));
	serial_print("looking for: 0x%x\n", type);
	
	struct multiboot_tag *header;
	uint8_t *curr = from;

	while ((uintptr_t)curr & 7) ++curr;

	do
	{
		header = (struct multiboot_tag *)(curr);		
		
		serial_print("curr header: 0x%6x ", V2P(header));
		serial_print("type: 0x%2x ", header->type);
		serial_print("size: 0x%6x ", header->size);
		serial_print("\n");

		if (header->type == type)
		{
			return curr;
		}

		curr += header->size;
		curr = (void *)(((uintptr_t)curr + 7) & ~7); // align up to the next 8-byte boundary

	} while (header->type != MULTIBOOT_TAG_TYPE_END);

	return NULL;
}

static void
mb2_init(uint32_t mbi_phys)
{
	struct multiboot2_start_tag *mbi_start = (struct multiboot2_start_tag *)(P2V(mbi_phys));
	serial_print("mbi size: 0x%x\n", mbi_start->size);
	serial_print("mbi reserved: 0x%x\n", mbi_start->reserved);

	/* TODO:
	 *  - pmm_init(memory_map);
	 *  - paging_reload_kernel_map();
	 * */

	struct multiboot_tag_mmap *mmap_tag = mb2_find_tag((void*)mbi_start + sizeof(struct multiboot2_start_tag), MULTIBOOT_TAG_TYPE_MMAP); 

	if (mmap_tag == NULL)
	{
		serial_print("[PANIC] Multiboot2: could not find mmap tag!\n");	
		while (1);
	}

	/* don't have a pmm yet, so how do we alloc new stuff using just the memory map?
	 * also need to make it "safe" from being cannibalized by the pmm later  
	 * so and I don't want to make a new entry
	 *
	 * a bit of cleverness
	 * find [	USED ENTRY	][		FREE ENTRY		] pair and grow used and shrink free
	 *			[ USED ENTRY	|	NEW	][ FREE ENTRY	]
	 *
	 * NEW =
	 * */

	//uint64_t map_pages = (mbi_start->size / PAGING_PAGE_SIZE) + 1; // 1 ?
	uint64_t map_pages = (mmap_tag->size / PAGING_PAGE_SIZE) + 1;
	serial_print("map_pages: 0x%x\n", map_pages);

	struct multiboot_mmap_entry *free_entry = NULL;
	struct multiboot_mmap_entry *used_entry = NULL;

	for (uint64_t i = 0; i * mmap_tag->entry_size < mmap_tag->size; ++i)
	{
		serial_print("entry: 0x%3x", i);
		serial_print("\t addr: 0x%10x", mmap_tag->entries[i].addr);
		serial_print("\t len: 0x%10x",  mmap_tag->entries[i].len);
		serial_print("\t type: 0x%3x", mmap_tag->entries[i].type);
		serial_print("\t zero: 0x%1x", mmap_tag->entries[i].zero);
		serial_print("\n");
	}

	for (uint64_t i = 1; i * mmap_tag->entry_size < mmap_tag->size; ++i)
	{
		if (mb2_mmap_type_convert(mmap_tag->entries[i-1].type) == MEMORY_MAP_BUSY
				&& mb2_mmap_type_convert(mmap_tag->entries[i].type) == MEMORY_MAP_FREE 
				&& (mmap_tag->entries[i].len / PAGING_PAGE_SIZE) >= map_pages)
		{
			used_entry = &mmap_tag->entries[i-1];
			free_entry = &mmap_tag->entries[i];
			break;
		}
	}

	if (free_entry == NULL || used_entry == NULL)
	{
		serial_print("[PANIC] Multiboot2: could not find mmap entry USED-FREE pair!\n");	
		while (1);
	}

	serial_print("used entry addr: 0x%10x\n", used_entry->addr);
	serial_print("free entry addr: 0x%10x\n", free_entry->addr);

	void *old_free_base = (void *)free_entry->addr;
	used_entry->len += map_pages * PAGING_PAGE_SIZE;
	free_entry->addr += map_pages * PAGING_PAGE_SIZE;
	free_entry->len -= map_pages * PAGING_PAGE_SIZE;

	/* make the new memory map having length map_pages * PAGING_MAP_SIZE */
	memory_map = (struct memory_map *)old_free_base;
	memory_map->entries = (struct memory_map_entry *)((uint64_t)old_free_base + sizeof(struct memory_map));
	memory_map->entry_count = mmap_tag->size / mmap_tag->entry_size + 1;

	for (uint64_t i = 1; i * mmap_tag->entry_size < mmap_tag->size; ++i)
	{
		memory_map->entries[i].base = mmap_tag->entries[i].addr;
		memory_map->entries[i].length = mmap_tag->entries[i].len;
		memory_map->entries[i].signal = mb2_mmap_type_convert(mmap_tag->entries[i].type);
	}


	/* TODO framebuffer tag */

	/* */


}

int 
kmain(uint32_t magic, uint32_t mbi_phys)
{
	serial_print("\n\n---------- UMBRA KERNEL ----------\n");

	if (magic != (uint32_t)0x36d76289)
	{
		serial_print("[PANIC] invalid magic!\n");
		while (1);
	}

	idt_assemble();
	mb2_init(mbi_phys);

	serial_print("--- end kmain ---\n");
	while (1);
	
	/* yield to common_main()	(common for all architectures) ; some kernel/kernel.c or kernel/main.c ? */
	// return common_main();
	return 0;
}
