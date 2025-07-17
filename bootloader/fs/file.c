#include "file.h"
#include "fat32.h"
#include <lib/misc.h>
#include <mm/pmm.h>
#include <drivers/vga.h>
#include <drivers/serial.h>

struct filehandle *
fopen(struct partition *part, const char *path)
{
	struct filehandle *ret;	

	size_t clean_len = strlen(path) + 2; // beginning / and ending 0
	char *path_clean = ext_mem_alloc(clean_len); 

	if (path[0] != '/') {
		path_clean[0] = '/';
		strcpy(&path_clean[1], path);
	} else {
		strcpy(path_clean, path);
	}

	/* go through all fs options, which is just fat32 for now */

	if ((ret = fat32_open(part, path_clean)) != NULL) {
		goto success;	
	}

	return NULL;

success:
	ret->path = (char *)path_clean;
	ret->path_len = clean_len;
	return ret;
}

void
fclose(struct filehandle *fh) 
{
	if (fh->is_memfile && fh->readall == false) {
		memmap_free(fh->fd, fh->size);
	} else {
		fh->close(fh);
	}
	memmap_free(fh->path, fh->path_len);
	memmap_free(fh, sizeof(struct filehandle));
	fh = NULL; // might not be the responsibility of this function, but whatever I put it here for now
}

void
fread(struct filehandle *fh, void *buf, uint64_t loc, uint64_t count)
{
	if (fh->is_memfile) {
		memcpy(buf, fh->fd + loc, count);
	} else {
		fh->read(fh, buf, loc, count);	
	}
}

void *
freadall(struct filehandle *fh, uint32_t type)
{
	if (fh->is_memfile)
	{
		if (fh->readall)
			return fh->fd;

		memmap_alloc_range((uint64_t)(size_t)fh->fd, ALIGN_UP(fh->size, 4096), type, 0, true, false);
		fh->readall = true;
		return fh->fd;
	}
	else
	{
		void *ret = ext_mem_alloc_aligned(fh->size, type, PAGE_SIZE, false);	
		fh->read(fh, ret, 0, fh->size);
		fh->close(fh);
		fh->fd = ret;
		fh->readall = true;
		fh->is_memfile = true;
		return ret;
	}

	__builtin_unreachable();
}

