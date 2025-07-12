#ifndef __URI_H__
#define __URI_H__

#include <fs/file.h>
#include <drivers/disk.h>

struct filehandle *uri_open(disk_t *boot_disk, char *uri);

#endif // !__URI_H__
