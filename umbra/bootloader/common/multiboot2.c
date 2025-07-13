#include <common/multiboot2.h>
#include <common/config.h>
#include <common/uri.h>
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

	/* TODO literally everything */
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

	/* TODO tags */

	memmap_free(kernel_path, strlen(kernel_path) + 1);
	while (1);
	__builtin_unreachable();
}

