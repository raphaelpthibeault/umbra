#include <types.h>

int 
kmain(uint32_t magic, uint32_t mbi_phys)
{
	(void)magic;
	(void)mbi_phys;

	while (1);
	
	/* yield to common_main()		(common for all architectures) */
	// return common_main();
	return 0;
}
