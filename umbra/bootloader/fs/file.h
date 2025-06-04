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
	uint64_t   size;	
};

/* TODO
 * fopen
 * fclose
 * fread
 * fwrite
 * */


#endif // !__FILE_H__
