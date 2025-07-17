OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start)

SECTIONS
{
	. = 1M;
	physical_start = .;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.bootstrap)
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)	
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)	
	}
	
	.bss BLOCK(4K) : ALIGN(4K)
	{
		PROVIDE(bss_start = .);
		*(COMMON)
		*(.bss)	
		*(.stack)
	}
	physical_end = .;

}
