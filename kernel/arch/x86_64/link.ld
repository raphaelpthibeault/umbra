OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start)

kernel_physical_base = 1M;
kernel_virtual_base = 0xffffffff80000000;

SECTIONS
{
	. = kernel_physical_base + kernel_virtual_base;

	physical_start = .;

	.text : AT(ADDR(.text) - kernel_virtual_base)
	{
		*(.multiboot)
		*(.bootstrap)
		*(.text .text.*)
	}

	.rodata : AT(ADDR(.rodata) - kernel_virtual_base)
	{
		*(.rodata .rodata.*)	
	}

	.data : AT(ADDR(.data) - kernel_virtual_base)
	{
		*(.data .data.*)	
	}
	
	.bss : AT(ADDR(.bss) - kernel_virtual_base)
	{
		PROVIDE(bss_start = .);
		*(COMMON)
		*(.bss .bss.*)	
		/* *(.stack) */
	}

	virtual_end = .;
	physical_end = . - kernel_virtual_base;

	/DISCARD/ : { *(.comment) }
}
