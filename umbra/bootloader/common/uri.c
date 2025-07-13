#include <common/uri.h>
#include <fs/file.h>
#include <lib/misc.h>
#include <lib/partition.h>
#include <drivers/disk.h>
#include <drivers/serial.h>
#include <mm/pmm.h>

/* format: resource(root):/path */
static bool
uri_resolve(char *uri, char **resource, char **root, char **path)
{
	size_t len = strlen(uri) + 1;
	char *buf = ext_mem_alloc(len);
	memcpy(buf, uri, len);
	uri = buf; // leak of uri? Nope, it's kernel_path which is freed in boot()

	*resource = *root = *path = NULL;

	/* resource */
	for (size_t i = 0; ; ++i)
	{
		if (strlen(uri + i) < 1)
			return false;

		if (memcmp(uri + i, "(", 1) == 0)
		{
			*resource = uri;
			uri[i] = 0;
			uri += i + 1;
			break;
		}
	}

	/* root */
	for (size_t i = 0; ; ++i)
	{
		if (strlen(uri + i) < 3)
			return false;

		if (memcmp(uri + i, "):/", 3) == 0)
		{
			*root = uri;
			uri[i] = 0;
			uri += i + 3;
			break;
		}
	}

	/* path is what's left */
	if (*uri == 0)
		return false;
	*path = uri;

	return true;
}

static struct filehandle *
uri_dispatch_boot(disk_t *boot_disk, char *path)
{
	// just loop fopen calls the same as every other time I've used my fopen 
	struct filehandle *fh;
	for (int i = 0; i < boot_disk->max_partition; ++i)
	{
		struct partition *part = &boot_disk->partition[i];
		if ((fh = fopen(part, path)) != NULL)
		{
			return fh;
		}
	}

	return NULL;	
}

struct filehandle *
uri_open(disk_t *boot_disk, char *uri)
{
	struct filehandle *kernel_file;
	char *resource = NULL, *root = NULL, *path = NULL;

	if (!uri_resolve(uri, &resource, &root, &path))
	{
		return NULL;		
	}

	if (strcmp(resource, "boot") == 0)
		kernel_file = uri_dispatch_boot(boot_disk, path);
	else
	{
		serial_print("[PANIC] unsupported resource\n");
		while (1);
	}

	return kernel_file;
}

