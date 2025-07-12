#ifndef __MULTIBOOT2_H__
#define __MULTIBOOT2_H__

#include <drivers/disk.h>

noreturn void multiboot2_load(disk_t *boot_disk, char *config);

#endif // !__MULTIBOOT2_H__
