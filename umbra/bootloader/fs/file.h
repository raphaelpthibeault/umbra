#ifndef __FILE_H__
#define __FILE_H__

#include <types.h>
#include <lib/partition.h>

struct filehandle {
	bool is_memfile;
	bool readall;
	struct partition *part;
	char *path;
	size_t path_len;
	void *fd;
	void (*read)(void *fd, void *buf, uint64_t loc, uint64_t count);
	void (*close)(void *fd);
	uint64_t size;	
};

struct filehandle *fopen(struct partition *part, const char *path);
void fclose(struct filehandle *fh);
void fread(struct filehandle *fh, void *buf, uint64_t loc, uint64_t count);

#endif // !__FILE_H__
