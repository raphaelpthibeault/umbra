#include "partition.h"
#include <types.h>
#include <lib/misc.h>
#include <mm/pmm.h>
#include <drivers/vga.h>
#include <drivers/disk.h>


partition_map_t *partition_map_list = NULL;
uint32_t partition_map_list_idx = 0;


bool
is_valid_mbr(disk_t disk)
{
	// read 512
}


int
partition_iterate() 
{
	
	// how do I get the partition_map_list ???

	partition_map_t *partmap;
	//  void *ext_mem_alloc(size_t count);
	
	for (uint32_t i = 0; i < partition_map_list_idx; ++i) {
		
	}

	return 0;

}
