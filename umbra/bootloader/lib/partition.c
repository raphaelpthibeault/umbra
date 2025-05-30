#include "partition.h"
#include <types.h>
#include <lib/misc.h>
#include <mm/pmm.h>
#include <drivers/vga.h>
#include <drivers/disk.h>


partition_map_t *partition_map_list = NULL;
uint32_t partition_map_list_idx = 0;


