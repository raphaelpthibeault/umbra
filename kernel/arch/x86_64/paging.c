#include <arch/x86_64/paging.h>
#include <drivers/serial.h>

/* 
bootstrap paging: in bss:
pml4_table:
	.skip 4096
pdp_table:
	.skip 4096
pd_table:
	.skip 4096
pt_table:
	.skip 4096

can't say fuck it and map the whole thing without having the memory map
don't have the size, but per the spec do know the max

maybe in some reload.c?
need to undo the bootstrap code and reload cr3
*/

/* ! from bootstrap ! */
extern paging_table_t *pml4_table;
extern paging_table_t *pdp_table;
extern paging_table_t *pd_table;
extern paging_table_t *ppt_table;

extern char kernel_end[];

//extern paging_table_t *pml4;
paging_table_t* pml4;

void *
paging_map_page(void* virt, void* phys, uint16_t flags)
{
	
}

