#include <common/multiboot2.h>
#include <common/config.h>
#include <common/uri.h>
#include <common/relocation.h>
#include <lib/misc.h>
#include <types.h>
#include <fs/file.h>
#include <drivers/disk.h>
#include <drivers/serial.h>
#include <mm/pmm.h>

noreturn void
multiboot2_load(disk_t *boot_disk, char *config)
{
	char *kernel_path = NULL;
	struct filehandle *kernel_file = NULL;

	kernel_path = config_get_value(config, 0, "PATH");
	if (kernel_path == NULL)
	{
		serial_print("[PANIC] Invalid configuration, no kernel path found\n");
		while (1);
	}
	serial_print("Multiboot2: Config kernel path: '%s' \n", kernel_path);

	if ((kernel_file = uri_open(boot_disk, kernel_path)) == NULL)
	{
		serial_print("[PANIC] kernel file is null / not found\n");
		while (1);
	}
	serial_print("Multiboot2: Found executable: '%s'\n", kernel_path);

	uint8_t *kernel = freadall(kernel_file, MEMMAP_KERNEL_AND_MODULES);
	if (kernel == NULL)
	{
		serial_print("[PANIC] could not read kernel\n");
		while (1);
	}
	size_t kernel_file_size = kernel_file->size;
	fclose(kernel_file);

	serial_print("Multiboot2: Read kernel into memory\n");

	struct multiboot_header *header;
	for (size_t header_offset = 0; header_offset < MULTIBOOT_SEARCH; header_offset += MULTIBOOT_HEADER_ALIGN)
	{
		header = (void *)(kernel + header_offset);	

		if (header->magic == MULTIBOOT2_HEADER_MAGIC)
			break;
	}
	serial_print("Multiboot2: Found the header\n");

	if (header->magic != MULTIBOOT2_HEADER_MAGIC)
	{
		serial_print("[PANIC] Multiboot2: Invalid magic number!\n");
		while (1);
	}
	serial_print("Multiboot2: Magic is valid\n");

	if (header->magic + header->architecture + header->header_length + header->checksum)
	{
		serial_print("[PANIC] Multiboot2: Invalid header checksum!\n");
		while (1);
	}
	serial_print("Multiboot2: Checksum is valid\n");

	
	uint64_t entry_point = 0xffffffff; /* load kernel at this point if no entry tag */
	struct multiboot_header_tag_address *address_tag = NULL;
	struct multiboot_header_tag_framebuffer *fb_tag = NULL;
	bool has_reloc_header = false;
	struct multiboot_header_tag_relocatable reloc_tag = {0};
	
	for (struct multiboot_header_tag *tag = (struct multiboot_header_tag *)(header + 1);
			tag < (struct multiboot_header_tag *)((uintptr_t)header + header->header_length) && tag->type != MULTIBOOT_HEADER_TAG_END;
			tag = (struct multiboot_header_tag *)((uintptr_t)tag + ALIGN_UP(tag->size, MULTIBOOT_TAG_ALIGN)))
	{
		bool is_required = !(tag->flags & MULTIBOOT_HEADER_TAG_OPTIONAL);

		switch (tag->type)
		{
			case MULTIBOOT_HEADER_TAG_ADDRESS:
			{
				address_tag = (void *)tag;
				serial_print("[INFO] Multiboot2: Given address tag\n");
				break;
			}
			case MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS:
			{
				struct multiboot_header_tag_entry_address *entry_tag = (void *)tag;
				entry_point = entry_tag->entry_addr;
				serial_print("[INFO] Multiboot2: Entry_point changed to 0x%x\n", entry_point);
				break;
			}
			case MULTIBOOT_HEADER_TAG_FRAMEBUFFER:
			{
				fb_tag = (void *)tag;
				serial_print("[INFO] Multiboot2: Given framebuffer tag\n");
				break;
			}
			case MULTIBOOT_HEADER_TAG_RELOCATABLE:
			{
				reloc_tag = *((struct multiboot_header_tag_relocatable *)tag);
				has_reloc_header = true;
				serial_print("[INFO] Multiboot2: Given relocatable tag\n");
				break;
			}
			case MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST:
			{
				serial_print("[WARNING] Info Requests currently unsupported\n");
				break;
			}
			default:
			{
				if (is_required)
				{
					serial_print("[PANIC] Multiboot2: Unknown header tag!\n");
					while (1);
				}
				else
				{
					serial_print("[WARNING] Multiboot2: Unknown header tag!\n");
				}
				break;
			}
		}

	}

	serial_print("Multiboot2: Tag parsing done\n");

	struct relocation_range *ranges;
	uint64_t ranges_count = 1;
	if (address_tag != NULL)
	{
		size_t header_offset = (size_t)header - (size_t)kernel;	

		uintptr_t load_src, load_addr;

		if (address_tag->load_addr != (uint32_t)-1)
		{
			if (address_tag->load_addr > address_tag->header_addr)
			{
				serial_print("[PANIC] Multiboot2: Illegal load address!\n");
				while (1);
			}

			load_src = header_offset - (address_tag->header_addr - address_tag->load_addr);
			load_addr = address_tag->load_addr;
		}
		else
		{
			load_src = 0;
			load_addr = address_tag->load_addr - header_offset;
		}

		size_t load_size;
		if (address_tag->load_end_addr != 0)
		{
			load_size = address_tag->load_end_addr - load_addr;
		} else {
			load_size = kernel_file_size - load_src;
		}

		size_t bss_size = 0;
		if (address_tag->bss_end_addr != 0)
		{
			uintptr_t bss_addr = load_addr + load_size;
			if (address_tag->bss_end_addr < bss_addr)
			{
				serial_print("[PANIC] Multiboot2: Illegal bss end address!\n");
				while (1);
			}

			bss_size = address_tag->bss_end_addr - bss_addr;
		}

		size_t full_size = load_size + bss_size;
		void *relocated = ext_mem_alloc(full_size);
		memcpy(relocated, kernel + load_src, load_size);

		if (entry_point == 0xffffffff) 
		{
			serial_print("[PANIC] Multiboot2: Using address tag with non-specified entry tag!\n");
			while (1);
		}

		ranges = ext_mem_alloc(sizeof(struct relocation_range));
		ranges->relocation = (uintptr_t)relocated;
		ranges->target = load_addr;
		ranges->length = full_size;

		serial_print("[INFO] Multiboot2: Calculated relocation range from address tag: \n\trelocation = 0x%x\n", ranges->relocation);
		serial_print("\ttarget = 0x%x\n", ranges->target);
		serial_print("\tlength = 0x%x\n", ranges->length);
	}
	else
	{
		/* TODO */
		serial_print("[PANIC] Multiboot2: Booting without giving address tag is currently unsupported\n");
		while (1);
	}

	int64_t reloc_slide = 0;
	if (has_reloc_header)
	{
		bool reloc_ascend;
		uint64_t reloc_base;

		switch (reloc_tag.preference)
		{
			/* prefer loading at lowest available */
			case 0:
			case 1:
				reloc_ascend = true;
				reloc_base = ALIGN_UP(reloc_tag.min_addr, reloc_tag.align);
				if (reloc_base + ranges->length > reloc_tag.max_addr)
				{
					goto reloc_fail;
				}
				break;
			/* prefer loading at highest available (will not support this) */
			default:
			case 2:
				serial_print("[PANIC] Multiboot2: Relocations with preferred loading at highest available is unsupported\n");
				while (1);
		}

		while (1)
		{
			if (check_usable_memory(reloc_base, reloc_base + ranges->length))
			{
				serial_print("Multiboot2: Found suitable relocation: \n\tbase: 0x%x\n", reloc_base);
				serial_print("\ttop: 0x%x\n", reloc_base + ranges->length);
				break;
			}

			if (reloc_ascend)
			{
				reloc_base += reloc_tag.align;
				if (reloc_base + ranges->length > reloc_tag.max_addr)
				{
					goto reloc_fail;
				}
			}
			else
			{
				reloc_base -= reloc_tag.align;
				if (reloc_base < reloc_tag.min_addr)
				{
					goto reloc_fail;
				}
			}
		}

		reloc_slide = (int64_t)reloc_base - ranges->target;
		entry_point += reloc_slide;
		ranges->target = reloc_base;
	}
	
	if (!check_usable_memory(ranges->target, ranges->target + ranges->length))
	{
reloc_fail:
		serial_print("[PANIC] Multiboot2: Relocation failed, could not find suitable address\n");
		while (1);
	}
	else
	{
		serial_print("Multiboot2: Will relocate to: \n\tbase: 0x%x\n", ranges->target);
		serial_print("\ttop: 0x%x\n", ranges->target + ranges->length);
	}

	/* TODO modules */

	uint64_t load_base_addr = ranges->target;



	memmap_free(kernel_path, strlen(kernel_path) + 1);
	while (1);
	__builtin_unreachable();
}

