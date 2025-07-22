#include <common/multiboot2.h>
#include <common/config.h>
#include <common/uri.h>
#include <common/relocation.h>
#include <common/acpi.h>
#include <common/terminal.h>
#include <common/panic.h>
#include <lib/misc.h>
#include <lib/elf.h>
#include <lib/framebuffer.h>
#include <types.h>
#include <fs/file.h>
#include <drivers/disk.h>
#include <drivers/serial.h>
#include <mm/pmm.h>

#define UMBRA_BRAND "Umbra" // should probably add a version
#define MEMMAP_MAX 256
#define DHCP_ACK_PACKET_LEN 296 /* I won't support PXE boot but maybe? Anyways place it here for the network tag info */

#define append_tag(P, TAG) do { \
    (P) += ALIGN_UP((TAG)->size, MULTIBOOT_TAG_ALIGN); \
} while (0)

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
		panic("Invalid configuration, no kernel path found");
	}
	serial_print("Multiboot2: Config kernel path: '%s' \n", kernel_path);

	if ((kernel_file = uri_open(boot_disk, kernel_path)) == NULL)
	{
		panic("kernel file is null / not found");
	}
	serial_print("Multiboot2: Found executable: '%s'\n", kernel_path);

	void *kernel = freadall(kernel_file, MEMMAP_KERNEL_AND_MODULES);
	if (kernel == NULL)
	{
		panic("could not read kernel");
	}
	size_t kernel_file_size = kernel_file->size;
	fclose(kernel_file);
	memmap_free(kernel_path, strlen(kernel_path) + 1);

	serial_print("Multiboot2: Read kernel into memory, file size 0x%x\n", kernel_file_size);

	struct multiboot_header *header = find_header(kernel, kernel_file_size < MULTIBOOT_SEARCH ? kernel_file_size : MULTIBOOT_SEARCH);
	if (header == NULL)
	{
		panic("Multiboot2: Could not find the multiboot2 header!");
	}
	serial_print("Multiboot2: Valid multiboot2 header\n");
	
	uint64_t entry_point = 0xffffffff; /* load kernel at this point if no entry tag */
	struct multiboot_header_tag_address *address_tag = NULL;
	struct multiboot_header_tag_framebuffer *fb_tag = NULL;
	bool has_reloc_header = false;
	struct multiboot_header_tag_relocatable reloc_tag = {0};
	
	bool is_new_acpi_required = false;
	bool is_old_acpi_required = false;
	bool is_elf_info_requested = false;

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
				struct multiboot_header_tag_information_request *request = (void *)tag;
				uint32_t size = (request->size - sizeof(struct multiboot_header_tag_information_request)) / sizeof(uint32_t);

				for (uint32_t i = 0; i < size; ++i)
				{
					uint32_t req = request->requests[i];
					switch (req)
					{
						case MULTIBOOT_TAG_TYPE_ACPI_NEW:
							is_new_acpi_required = is_required;
							break;
						case MULTIBOOT_TAG_TYPE_ACPI_OLD:
							is_old_acpi_required = is_required;
							break;
						case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
							is_elf_info_requested = is_required;
							break;
						default:
							if (is_required)
							{
								panic("Multiboot2: Requested unsupported tag %d", req);	
							}
					}
				}
				break;
			}
			default:
			{
				if (is_required)
				{
					panic("Multiboot2: Unknown header tag!");
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
				panic("Multiboot2: Illegal load address!");
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
				panic("Multiboot2: Illegal bss end address!");
			}

			bss_size = address_tag->bss_end_addr - bss_addr;
		}

		size_t full_size = load_size + bss_size;
		void *relocated = ext_mem_alloc(full_size);
		memcpy(relocated, kernel + load_src, load_size);

		if (entry_point == 0xffffffff) 
		{
			panic("Multiboot2: Using address tag with non-specified entry tag!");
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
			panic("ELF load relocation failure");
		}

		shdr_info = elf64_get_elf_shdr_info(kernel);
		shdr_info_valid = true;

		if (entry_point == 0xffffffff) 
		{
			entry_point = elf_entry;
		}

		serial_print("[INFO] Multiboot2: ELF entry (physical): 0x%x\n", entry_point);
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
				panic("Multiboot2: Relocations with preferred loading at highest available is unsupported");
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
		panic("Multiboot2: Relocation failed, could not find suitable address");
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
	if (!relocation_append(ranges, &ranges_count, mb2_info, &mb2_info_final_loc, mb2_info_size))
	{
		panic("Could not allocate mb2 info!");	
	}

	serial_print("[DEBUG] ranges_count: 0x%x\n", ranges_count); // should be 2
	serial_print("[DEBUG] mb2_info_final_loc: 0x%x\n", mb2_info_final_loc);

	size_t mbi_idx = 0;
	struct multiboot2_start_tag *mbi_start = (struct multiboot2_start_tag *)(mb2_info);
	mbi_idx += sizeof(struct multiboot2_start_tag);

	/* ELF info tag */
	if (!shdr_info_valid)
	{
		if (is_elf_info_requested)
		{
			panic("Multiboot2: Requested ELF info, but has invalid ELF section header!");
		}
	}
	else
	{
		struct multiboot_tag_elf_sections *tag = (struct multiboot_tag_elf_sections*)(mb2_info + mbi_idx);

		tag->type = MULTIBOOT_TAG_TYPE_ELF_SECTIONS;
		tag->size = sizeof(struct multiboot_tag_elf_sections) + shdr_info.section_entry_size * shdr_info.num;
		tag->num = shdr_info.num;
		tag->entsize = shdr_info.section_entry_size;
		tag->shndx = shdr_info.str_section_idx;

		memcpy(tag->sections, kernel + shdr_info.section_offset, shdr_info.section_entry_size * shdr_info.num);

		for (size_t i = 0; i < shdr_info.num; ++i)
		{
			struct elf64_shdr *shdr = (void *)tag->sections + i * shdr_info.section_entry_size;

			if (shdr->addr != 0 || shdr->size == 0)
			{
				continue;
			}

			uint64_t section = (uint64_t)-1; /* specify no target preference (go top-down) */
			if (!relocation_append(ranges, &ranges_count, kernel + shdr->offset, &section, shdr->size))
			{
				panic("Could not allocate ELF info!");	
			}

			shdr->addr = section;
		}

		append_tag(mbi_idx, tag);
	}
	serial_print("[DEBUG] appended ELF tag maybe?\n");	

	/* load base address tag */
	if (has_reloc_header)
	{
		struct multiboot_tag_load_base_addr *tag = (struct multiboot_tag_load_base_addr *)(mb2_info + mbi_idx);

		tag->type = MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR;
		tag->size = sizeof(struct multiboot_tag_load_base_addr);
		tag->load_base_addr = load_base_addr;

		append_tag(mbi_idx, tag);
	}
	serial_print("[DEBUG] appended load base address tag maybe?\n");	

	/* TODO modules tag */

	/* TODO command line tag */

	/* TODO bootloader name tag */

	{
		terminal_deinit();
		struct multiboot_tag_framebuffer *tag = (struct multiboot_tag_framebuffer *)(mb2_info + mbi_idx);	

		tag->common.type = MULTIBOOT_TAG_TYPE_FRAMEBUFFER;
		tag->common.size = sizeof(struct multiboot_tag_framebuffer);

		if (fb_tag == NULL)
		{
			/* TODO text mode */
			panic("Multiboot2: Non-framebuffer boot is currently unsupprted!");
		}

		size_t req_width = 0, req_height = 0, req_bpp = 0;
		struct fb_info *fb = NULL; 
		size_t fb_count = 0;
		
		req_width = fb_tag->width;
		req_height = fb_tag->height;
		req_bpp = fb_tag->depth;

		if (req_width == 0 || req_height == 0 || req_bpp == 0)
		{
			serial_print("[DEBUG] [WARNING] Gave a framebuffer tag but with zeroed property(ies). The bootloader will decide framebuffer.\n");
		}

		fb = fb_init(&fb_count, req_width, req_height, req_bpp);

		if (fb == NULL || fb_count == 0) 
		{
			/* TODO: maybe set text mode here instead of panicking? */
			panic("Multiboot2: Failed to set video mode!");
		}

		tag->common.framebuffer_addr = fb->framebuffer_addr;
		tag->common.framebuffer_pitch = fb->framebuffer_pitch;
		tag->common.framebuffer_width = fb->framebuffer_width;
		tag->common.framebuffer_height = fb->framebuffer_height;
		tag->common.framebuffer_bpp = fb->framebuffer_bpp;
		tag->common.framebuffer_type = MULTIBOOT_FRAMEBUFFER_TYPE_RGB; // I only supported RGB for VBE

		tag->framebuffer_red_field_position = fb->red_mask_shift;
		tag->framebuffer_red_mask_size = fb->red_mask_size;
		tag->framebuffer_green_field_position = fb->green_mask_shift;
		tag->framebuffer_green_mask_size = fb->green_mask_size;
		tag->framebuffer_blue_field_position = fb->blue_mask_shift;
		tag->framebuffer_blue_mask_size = fb->blue_mask_size;

		append_tag(mbi_idx, &tag->common);
	}
	serial_print("[DEBUG] appended framebuffer tag maybe?\n");	

	/* TODO new ACPI info tag */

	/* TODO old ACPI info tag */

	/* TODO SMBIOS tag */

	/* TODO relocation stub ? */

	/* >>> TODO memory map tag */

	/* >>> TODO basic memory info tag */

	/* TODO network info tag */
	
	/* end tag */
	{
		struct multiboot_tag *end_tag = (struct multiboot_tag *)(mb2_info + mbi_idx);

		end_tag->type = MULTIBOOT_TAG_TYPE_END;
		end_tag->size = sizeof(struct multiboot_tag);

		append_tag(mbi_idx, end_tag);
	}
	serial_print("[DEBUG] appended end tag maybe?\n");	

	/* all tags done */
	mbi_start->size = mbi_idx;
	mbi_start->reserved = 0x00;

	/* TODO IRQ flush PIC */
	/* TODO spinup(); */

	while (1);
	__builtin_unreachable();
}

