#include <common/multiboot2.h>
#include <common/config.h>
#include <common/uri.h>
#include <common/relocation.h>
#include <common/acpi.h>
#include <lib/misc.h>
#include <lib/elf.h>
#include <types.h>
#include <fs/file.h>
#include <drivers/disk.h>
#include <drivers/serial.h>
#include <mm/pmm.h>

#define UMBRA_BRAND "Umbra" // should probably add a version
#define MEMMAP_MAX 256
#define DHCP_ACK_PACKET_LEN 296 /* I won't support PXE boot but maybe? Anyways place it here for the network tag info */

static size_t
get_mb2_info_size(
		char *cmdline,
		size_t modules_size,
		uint32_t section_entry_size, 
		uint32_t section_nb,
		uint32_t smbios_tag_size)
{
	return	ALIGN_UP(sizeof(struct multiboot2_start_tag), MULTIBOOT_TAG_ALIGN) +																						// start
					ALIGN_UP(sizeof(struct multiboot_tag_string) + strlen(cmdline) + 1, MULTIBOOT_TAG_ALIGN) +                      // cmdline
					ALIGN_UP(sizeof(struct multiboot_tag_string) + sizeof(UMBRA_BRAND), MULTIBOOT_TAG_ALIGN) +											// bootloader brand
					ALIGN_UP(sizeof(struct multiboot_tag_framebuffer), MULTIBOOT_TAG_ALIGN) +                                       // framebuffer
					ALIGN_UP(sizeof(struct multiboot_tag_new_acpi) + sizeof(struct rsdp), MULTIBOOT_TAG_ALIGN) +                    // new ACPI info
					ALIGN_UP(sizeof(struct multiboot_tag_old_acpi) + 20, MULTIBOOT_TAG_ALIGN) +                                     // old ACPI info
					ALIGN_UP(sizeof(struct multiboot_tag_elf_sections) + section_entry_size * section_nb, MULTIBOOT_TAG_ALIGN) +		// ELF info
					ALIGN_UP(modules_size, MULTIBOOT_TAG_ALIGN) +                                                                   // modules
					ALIGN_UP(sizeof(struct multiboot_tag_load_base_addr), MULTIBOOT_TAG_ALIGN) +                                    // load base address
					ALIGN_UP(smbios_tag_size, MULTIBOOT_TAG_ALIGN) +                                                                // SMBIOS
					ALIGN_UP(sizeof(struct multiboot_tag_basic_meminfo), MULTIBOOT_TAG_ALIGN) +                                     // basic memory info
					ALIGN_UP(sizeof(struct multiboot_tag_mmap) + sizeof(struct multiboot_mmap_entry) * MEMMAP_MAX, MULTIBOOT_TAG_ALIGN) +  // MMAP
					ALIGN_UP(sizeof(struct multiboot_tag_network) + DHCP_ACK_PACKET_LEN, MULTIBOOT_TAG_ALIGN) +											// network info
					ALIGN_UP(sizeof(struct multiboot_tag), MULTIBOOT_TAG_ALIGN);                                                    // end
}

static struct multiboot_header *
find_header(uint8_t *buffer, size_t len)
{
	struct multiboot_header *header;
	/* The header should be at least 12 bytes and aligned on a 4-byte boundary */
	for (header = (struct multiboot_header *)buffer;
			((uint8_t *)header <= buffer + len - 12);
			header = (struct multiboot_header *)((uint32_t *)header + MULTIBOOT_HEADER_ALIGN / 4))
	{
		if (header->magic == MULTIBOOT2_HEADER_MAGIC 
				&& !(header->magic + header->architecture + header->header_length + header->checksum))
		{
			return header;
		}
	}

	return NULL;
}


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

	void *kernel = freadall(kernel_file, MEMMAP_KERNEL_AND_MODULES);
	if (kernel == NULL)
	{
		serial_print("[PANIC] could not read kernel\n");
		while (1);
	}
	size_t kernel_file_size = kernel_file->size;
	fclose(kernel_file);
	memmap_free(kernel_path, strlen(kernel_path) + 1);

	serial_print("Multiboot2: Read kernel into memory, file size 0x%x\n", kernel_file_size);

	struct multiboot_header *header = find_header(kernel, kernel_file_size < MULTIBOOT_SEARCH ? kernel_file_size : MULTIBOOT_SEARCH);
	if (header == NULL)
	{
		serial_print("[PANIC] Multiboot2: Could not find the multiboot2 header!\n");
		while (1);
	}
	serial_print("Multiboot2: Valid multiboot2 header\n");
	
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

	serial_print("Multiboot2: Tag lexing done\n");

	struct relocation_range *ranges;
	uint64_t ranges_count = 1;
	bool shdr_info_valid = false;
	struct elf_shdr_info shdr_info = {0};
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
		uint64_t elf_entry;

		serial_print("Multiboot2: No address tag found, proceeding with ELF loading.\n");

		if (!elf64_load_relocation(kernel, &elf_entry, &ranges))
		{
			serial_print("[PANIC] ELF load relocation failure\n");
			while (1);
		}

		shdr_info = elf64_get_elf_shdr_info(kernel);
		shdr_info_valid = true;

		if (entry_point == 0xffffffff) 
		{
			entry_point = elf_entry;
		}

		serial_print("[INFO} Multiboot2: ELF entry (physical): 0x%x\n", entry_point);
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
		serial_print("Multiboot2: Will relocate kernel to: \n\tbase: 0x%x\n", ranges->target);
		serial_print("\ttop: 0x%x\n", ranges->target + ranges->length);
	}
	uint64_t load_base_addr = ranges->target;

	/* TODO modules */
	size_t modules_size = 0;
	size_t n_modules = 0;

	struct smbios_entry_point_32 *smbios_eps_32 = NULL;
	struct smbios_entry_point_64 *smbios_eps_64 = NULL;

	acpi_get_smbios((void **)&smbios_eps_32, (void **)&smbios_eps_64);
	uint32_t smbios_tag_size = 0;

	if (smbios_eps_32 != NULL)
	{
		smbios_tag_size += sizeof(struct multiboot_tag_smbios) + smbios_eps_32->entry_point_length;
	}
	if (smbios_eps_64 != NULL)
	{
		smbios_tag_size += sizeof(struct multiboot_tag_smbios) + smbios_eps_64->entry_point_length;
	}

	/* realloc relocation ranges to include mb2 info, modules, and elf sections */
	struct relocation_range *new_ranges = ext_mem_alloc(sizeof(struct relocation_range) * 
				(ranges_count 
				 + 1 /* mb2 info range */
				 + n_modules
				 + (shdr_info_valid ? shdr_info.num : 0)));

	memcpy(new_ranges, ranges, sizeof(struct relocation_range) * ranges_count);
	memmap_free(ranges, sizeof(struct relocation_range) * ranges_count);
	ranges = new_ranges;

	/* TODO cmdline config option */
	char *cmdline = "";
	size_t mb2_info_size = get_mb2_info_size(
			cmdline,
			modules_size,
			shdr_info_valid ? shdr_info.section_entry_size : 0,
			shdr_info_valid ? shdr_info.num : 0,
			smbios_tag_size
	);

	/* from what I can tell, GRUB allocates boot info at 0x10000, so let's yoink that idea */
	/* append mb2 info after kernel but before modules */
	uint8_t *mb2_info = ext_mem_alloc(mb2_info_size);
	uint64_t mb2_info_final_loc = 0x10000;
	//bool relocation_append(struct relocation_range *ranges, uint64_t *ranges_count, void *relocation, uint64_t *target, size_t length);
	if (!relocation_append(ranges, &ranges_count, mb2_info, &mb2_info_final_loc, mb2_info_size))
	{
		serial_print("[PANIC] Could not allocate mb2 info!\n");	
		while (1);
	}

	serial_print("[DEBUG] ranges_count: 0x%x\n", ranges_count); // should be 2
	serial_print("[DEBUG] mb2_info_final_loc: 0x%x\n", mb2_info_final_loc);


	// terminal_deinit(); // done
	// spinup();
	//
	while (1);
	__builtin_unreachable();
}

