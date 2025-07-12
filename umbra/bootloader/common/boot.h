#ifndef __BOOT_H__
#define __BOOT_H__

#include <types.h>
#include <fs/file.h>
#include <drivers/disk.h>

noreturn void boot(disk_t *boot_disk, char *config);

#endif // !__BOOT_H__
