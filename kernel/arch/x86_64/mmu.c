#include <types.h>
#include <arch/x86_64/paging.h>
#include <arch/x86_64/memory_references.h>
#include <arch/x86_64/mmu.h>
#include <drivers/serial.h>
#include <protocol.h>

/*
 * Memory Management Unit
 * */
#define INDEX_FROM_BIT(b)  ((b) >> 5)
#define OFFSET_FROM_BIT(b) ((b) & 0x1F)

#define KERNEL_HEAP_START 0xfffffff000000000UL

extern char kernel_end[];

/* bootstrap paging tables */
extern paging_table_t *pml4_table;
extern paging_table_t *pdp_table;
extern paging_table_t *pd_table;
extern paging_table_t *pt_table;

/* global state */
static volatile uint32_t *frames; /* bitmap page frame allocator */
static size_t num_frames;
static uintptr_t lowest_available_frame = 0; /* first frame and first n frames */
static size_t total_memory = 0;
static size_t unavailable_memory = 0;

static void
unmark_available_memory(struct memory_map memory_map)
{
	for (size_t i = 0; i < memory_map.entry_count; ++i)
	{
		struct memory_map_entry this = memory_map.entries[i];
		if (this.type == MEMORY_MAP_MEMORY_AVAILABLE)
		{
			for (uintptr_t base = this.base; base < this.base + (this.length & 0xfffffffffffff000); base += PAGING_PAGE_SIZE)
			{
				mmu_frame_clear(base);	
			}
		}
	}
}

void 
mmu_init(struct memory_map memory_map)
{
	size_t free_memory = 0;
	size_t first_free_page = 0;
	for (size_t i = 0; i < memory_map.entry_count; ++i)
	{
		struct memory_map_entry this = memory_map.entries[i];

		serial_print("entry: 0x%3x", i);
		serial_print("\t addr: 0x%10x", this.base);
		serial_print("\t len: 0x%10x",  this.length);
		serial_print("\t type: 0x%3x",  this.type);
		serial_print("\t zero: 0x%1x",  this.zero);
		serial_print("\n");

		if (this.type == MEMORY_MAP_MEMORY_AVAILABLE)
		{
			free_memory += this.length;
		}
	}
	serial_print("[INFO] free_memory = 0x%x\n", free_memory);

	/* round the maximum address up a page
	 * shouldn't this be based on modules ??
	 * regardless, this is supposed to be a physical address 
	 * */
	first_free_page = V2P((uintptr_t)kernel_end);
	first_free_page = (first_free_page + PAGING_PAGE_MASK) & 0xfffffffffffff000UL;
	serial_print("[INFO] first free page = 0x%x\n", first_free_page);

	/* enable the Write Protect (WP) bit 
	 * will cause page faults upon kernel writes to non-writable pages 
	 *
	 * NOTE:
	 * this is a future-proof sort of thing for Copy-On-Write mappings for user processes that are obviously todo right now
	 * */

	__asm__ volatile (
		"movq %%cr0, %%rax\n"
		"orq  $0x10000, %%rax\n"
		"movq %%rax, %%cr0\n"
		: : : "rax");

	/* bootstrap paging tables map bios and kernel; everything else is unmapped (for instance kernel heap)
	 * current restriction: can only map 2 MiB (there's 1 pt_table)
	 * using bootstrap paging tables, map first_free_page manually 
	 * 
	 * how much memory do we need?
	 * enough for a bitmap, and a page structure
	 * pmm_alloc_page
	 * should return first_free_page by coincidence
	 * */

	num_frames = free_memory >> PAGING_PAGE_BITS;
	serial_print("num_frames: 0x%x\n", num_frames);
	size_t bytes_of_frames = INDEX_FROM_BIT(num_frames * 8);
	bytes_of_frames = (bytes_of_frames + PAGING_PAGE_MASK) & 0xfffffffffffff000UL;
	serial_print("bytes_of_frames: 0x%x\n", bytes_of_frames);
	bytes_of_frames = (bytes_of_frames + PAGING_PAGE_MASK) & 0xfffffffffffff000UL;
	size_t pages_of_frames = bytes_of_frames >> PAGING_PAGE_BITS;
	serial_print("pages_of_frames: 0x%x\n", pages_of_frames);

	/* find place to store pmm data : should be first_free_page */
	paging_indexer_t indexer;
	paging_indexer_assign(&indexer, (void *)((uintptr_t)P2V(first_free_page)));

	serial_print("first free page:\n");
	serial_print("\tpml4 idx: 0x%x\n", indexer.pml4);
	serial_print("\tpdp idx: 0x%x\n", indexer.pdp);
	serial_print("\tpd idx: 0x%x\n", indexer.pd);
	serial_print("\tpt idx: 0x%x\n", indexer.pt);

	frames = (void *)((uintptr_t)P2V(first_free_page));

	/*  idea: mark all page frames, then unmark the "available" ones 
	 * */

	for (size_t i = 0; i < bytes_of_frames; ++i)
	{
		frames[i] = 0xff;
	}

	unmark_available_memory(memory_map);

	size_t unavailable = 0, available = 0;
	for (size_t i = 0; i < INDEX_FROM_BIT(num_frames); ++i)
	{
		for (size_t j = 0; j < 32; ++j)
		{
			uint32_t test_frame = (uint32_t)0x1 << j;
			if (frames[i] & test_frame)
			{
				++unavailable;
			}
			else
			{
				++available;
			}
		}
	}

	total_memory = available * 4;
	unavailable_memory = unavailable * 4;

	/* mark everything up to (first_free_page + bytes_of_frames as in use) */
	/* includes: kernel + bitmap */
	for (uintptr_t i = 0; i < first_free_page + bytes_of_frames; i += PAGING_PAGE_SIZE)
	{
		mmu_frame_set(i);	
	}

	/* tests */
	paging_indexer_t foo;
	paging_indexer_assign(&foo, (void *)((uintptr_t)KERNEL_HEAP_START));

	serial_print("kernel heap start maybe?:\n");
	serial_print("\tpml4 idx: 0x%x\n", foo.pml4);
	serial_print("\tpdp idx:  0x%x\n", foo.pdp);
	serial_print("\tpd idx:   0x%x\n", foo.pd);
	serial_print("\tpt idx:   0x%x\n", foo.pt);

	paging_indexer_assign(&foo, (void *)((uintptr_t)0xffffff0000000000ul));

	serial_print("alternative kernel heap start maybe?:\n");
	serial_print("\tpml4 idx: 0x%x\n", foo.pml4);
	serial_print("\tpdp idx:  0x%x\n", foo.pdp);
	serial_print("\tpd idx:   0x%x\n", foo.pd);
	serial_print("\tpt idx:   0x%x\n", foo.pt);
}

void 
mmu_frame_clear(uintptr_t frame_addr)
{
	if (frame_addr < num_frames * PAGING_PAGE_SIZE)
	{
		uint64_t frame = frame_addr >> PAGING_PAGE_BITS;
		uint64_t index = INDEX_FROM_BIT(frame);
		uint32_t offset = OFFSET_FROM_BIT(frame);
		frames[index] &= ~((uint32_t)1 << offset);
		__asm__("" ::: "memory");
		if (frame < lowest_available_frame)
		{
			lowest_available_frame = frame;
		}
	}
}

void 
mmu_frame_set(uintptr_t frame_addr)
{
	if (frame_addr < num_frames * PAGING_PAGE_SIZE)
	{
		uint64_t frame = frame_addr >> PAGING_PAGE_BITS;
		uint64_t index = INDEX_FROM_BIT(frame);
		uint32_t offset = OFFSET_FROM_BIT(frame);
		frames[index] |= ((uint32_t)1 << offset);
		__asm__("" ::: "memory");
	}
}


/* -------------------- reload -------------------- */


