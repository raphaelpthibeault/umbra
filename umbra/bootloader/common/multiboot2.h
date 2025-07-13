#ifndef __MULTIBOOT2_H__
#define __MULTIBOOT2_H__

#include <drivers/disk.h>
#include <types.h>

#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

struct multiboot2_header
{
	uint32_t magic;
	uint32_t arch;
	uint32_t header_length;
	uint32_t checksum;
};


noreturn void multiboot2_load(disk_t *boot_disk, char *config);

#endif // !__MULTIBOOT2_H__
