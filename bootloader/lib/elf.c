#include <lib/elf.h>
#include <lib/misc.h>
#include <types.h>
#include <drivers/serial.h>
#include <mm/pmm.h>

#define ARCH_X86_64  0x3e
#define ARCH_X86_32  0x03

#define ET_NONE     0
#define ET_REL      1
#define ET_EXEC     2
#define ET_DYN      3

#define PT_LOAD     0x00000001
#define PT_DYNAMIC  0x00000002
#define PT_INTERP   0x00000003
#define PT_PHDR     0x00000006

static bool
elf64_validate(struct elf64_ehdr *ehdr)
{
	if (ehdr->ident[EI_MAG0] != ELFMAG0
			|| ehdr->ident[EI_MAG1] != ELFMAG1
			|| ehdr->ident[EI_MAG2] != ELFMAG2
			|| ehdr->ident[EI_MAG3] != ELFMAG3)
	{
		serial_print("ehdr->ident[0]: 0x%x\n", ehdr->ident[EI_MAG0]);
		serial_print("ehdr->ident[1]: 0x%x\n", ehdr->ident[EI_MAG1]);
		serial_print("ehdr->ident[2]: 0x%x\n", ehdr->ident[EI_MAG2]);
		serial_print("ehdr->ident[3]: 0x%x\n", ehdr->ident[EI_MAG3]);
		serial_print("[PANIC] ELF: Invalid ELF64 magic!\n");
		return false;
	}
	if (ehdr->machine != ARCH_X86_64)
	{
		serial_print("[PANIC] ELF: Machine not x86_64!\n");
		return false;
	}

	return true;
}

struct elf_shdr_info 
elf64_get_elf_shdr_info(uint8_t *elf)
{
	struct elf_shdr_info info = {0};

	struct elf64_ehdr *ehdr = (struct elf64_ehdr *)(elf);

	if (!elf64_validate(ehdr))
	{
		serial_print("[PANIC] elf64_validate failure\n");
		while (1);
	}

	info.num = ehdr->shnum;
	info.section_entry_size = ehdr->shentsize;
	info.str_section_idx = ehdr->shstrndx;
	info.section_offset = ehdr->shoff;

	return info;
}


bool
elf64_load_relocation(uint8_t *elf, uint64_t *entry_point, struct relocation_range **ranges)
{
		struct elf64_ehdr *ehdr = (struct elf64_ehdr *)(elf);

		if (!elf64_validate(ehdr))
		{
			serial_print("[PANIC] elf64_validate failure\n");
			while (1);
		}
		if (ehdr->type != ET_EXEC && ehdr->type != ET_DYN)
		{
			serial_print("[PANIC] ELF: ELF file not EXEC or DYN!\n");
			return false;
		}

		*entry_point = ehdr->entry; // VMA!
		bool entry_adjusted = false;

		if (ehdr->phentsize < sizeof(struct elf64_phdr))
		{
			serial_print("[PANIC] ELF: Program header table entry size < sizeof(struct elf64_phdr)!\n");
			while (1);
		}

		size_t image_size;
		uint64_t min_paddr = (uint64_t)-1;
		uint64_t max_paddr = 0;

		for (uint16_t i = 0; i < ehdr->phnum; ++i)
		{
			struct elf64_phdr *phdr = (void *)elf + (ehdr->phoff + i * ehdr->phentsize);

			if (phdr->type != PT_LOAD || phdr->memsz == 0)
			{
				continue;	
			}

			if (phdr->paddr < min_paddr)
			{
				min_paddr = phdr->paddr;
			}

			if (phdr->paddr + phdr->memsz > max_paddr)
			{
				max_paddr = phdr->paddr + phdr->memsz;
			}
		}

		image_size = max_paddr - min_paddr;

		void *relocation = ext_mem_alloc(image_size);
		*ranges = ext_mem_alloc(sizeof(struct relocation_range));
		(*ranges)->relocation = (uintptr_t)relocation;
		(*ranges)->target = min_paddr;
		(*ranges)->length = image_size;

		for (uint16_t i = 0; i < ehdr->phnum; ++i)
		{
			struct elf64_phdr *phdr = (void *)elf + (ehdr->phoff + i * ehdr->phentsize);

			if (phdr->type != PT_LOAD || phdr->memsz == 0)
			{
				continue;	
			}

			memcpy(relocation + (phdr->paddr - min_paddr), elf + phdr->offset, phdr->filesz);

			if (!entry_adjusted
					&& *entry_point >= phdr->vaddr
					&& *entry_point < (phdr->vaddr + phdr->memsz))
			{
				*entry_point -= phdr->vaddr; /* over-corrects, have to re-add physical address */
				*entry_point += phdr->paddr;
				entry_adjusted = true;
			}
		}

		return true;
}
