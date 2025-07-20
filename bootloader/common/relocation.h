#ifndef __COMMON_RELOCATION_H__
#define __COMMON_RELOCATION_H__

#include <types.h>

struct relocation_range
{
	uint64_t relocation;	
	uint64_t target;
	uint64_t length;
};

bool relocation_append(struct relocation_range *ranges, uint64_t *ranges_count, void *relocation, uint64_t *target, size_t length);

#endif // !__COMMON_RELOCATION_H__
