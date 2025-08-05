#include <types.h>
#include <multiboot2.h>
#include <protocol.h>
#include <elf.h>
#include <drivers/serial.h>

#include <arch/x86_64/idt.h>
#include <arch/x86_64/memory_references.h>
#include <arch/x86_64/paging.h>
#include <arch/x86_64/mmu.h>

/* 
 * main routine for x86_64
 * */

extern char kernel_end[];

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

static struct memory_map
mb2_init(uint32_t mbi_phys)
{
	struct memory_map memory_map = {0};

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

	memory_map.entry_count = (mmap_tag->size - sizeof(struct multiboot_tag_mmap)) / mmap_tag->entry_size;
	serial_print("memory_map entry count: 0x%3x\n", memory_map.entry_count);
	memory_map.entries = (void *)&mmap_tag->entries;


	return memory_map;
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
	struct memory_map memory_map = mb2_init(mbi_phys);
	mmu_init(memory_map);

	serial_print("--- end kmain ---\n");
	while (1);
	
	/* yield to common_main()	(common for all architectures) ; some kernel/kernel.c or kernel/main.c ? */
	// return common_main();
	return 0;
}
