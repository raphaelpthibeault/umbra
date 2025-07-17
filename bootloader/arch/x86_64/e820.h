#ifndef __E820_H__
#define __E820_H__

#include <mm/pmm.h>

extern struct memmap_entry e820_map[];
extern size_t e820_entries;

void do_e820(void);

#endif // !__E820_H__
