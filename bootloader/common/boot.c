#include <common/boot.h>
#include <types.h>
#include <lib/misc.h>
#include <fs/file.h>
#include <drivers/serial.h>
#include <mm/pmm.h>
#include <common/config.h>
#include <drivers/disk.h>
#include <common/multiboot2.h>

noreturn void
boot(disk_t *boot_disk, char *config)
{
	char *protocol;
	serial_print("Boot: Selected config: \n'''%s'''\n", config);
	protocol = config_get_value(config, 0, "PROTOCOL");

	serial_print("Boot: Config protocol: '%s'\n", protocol);
	if (strcmp(protocol, "multiboot2") == 0)
	{
		memmap_free(protocol, strlen(protocol) + 1);
		multiboot2_load(boot_disk, config);
	}
	else
	{
		serial_print("[PANIC] unsupported protocol\n");
		while (1);
	}

	__builtin_unreachable();
}
