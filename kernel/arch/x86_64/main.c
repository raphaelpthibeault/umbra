#include <types.h>
#include <multiboot2.h>
#include <protocol.h>
#include <elf.h>
#include <drivers/serial.h>

#include <arch/x86_64/idt.h>
#include <arch/x86_64/memory_references.h>
#include <arch/x86_64/paging.h>

/* 
 * main routine for x86_64
 * */

struct mb2_init
{
	uint64_t highest_valid_address;
	uint64_t highest_kernel_address;
};

extern void mmu_init(size_t memsize, uintptr_t first_free_page);

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

	struct multiboot_tag_elf_sections *elf_tag = mb2_find_tag((void*)mbi_start + sizeof(struct multiboot2_start_tag), MULTIBOOT_TAG_TYPE_ELF_SECTIONS); 
	serial_print("ELF_TAG: \n");
	serial_print("\tsize: 0x%x\n", elf_tag->size);
	serial_print("\tnum: 0x%x\n", elf_tag->num);
	serial_print("\tentsize: 0x%x\n", elf_tag->entsize);
	serial_print("\tshndx: 0x%x\n", elf_tag->shndx);

	uint8_t *sections_base = (uint8_t *)&elf_tag->sections;
	for (size_t i = 0; i < elf_tag->num; ++i)
	{
		struct elf64_shdr *shdr = (struct elf64_shdr *)(sections_base + i * elf_tag->entsize);
		(void)shdr;
	}

	//struct multiboot_tag_framebuffer *fb_tag = mb2_find_tag((void*)mbi_start + sizeof(struct multiboot2_start_tag), MULTIBOOT_TAG_TYPE_FRAMEBUFFER); 


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

	for (uint64_t i = 0; i * mmap_tag->entry_size < mmap_tag->size; ++i)
	{
		serial_print("entry: 0x%3x", i);
		serial_print("\t addr: 0x%10x", mmap_tag->entries[i].addr);
		serial_print("\t len: 0x%10x",  mmap_tag->entries[i].len);
		serial_print("\t type: 0x%3x", mmap_tag->entries[i].type);
		serial_print("\t zero: 0x%1x", mmap_tag->entries[i].zero);
		serial_print("\n");
	}

	/* why can't I just pmm_init with the multiboot memory map? */

	/* look for memory hole */

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
