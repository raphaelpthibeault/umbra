OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start)

kernel_physical_base = 1M;
kernel_virtual_base = 0xffffffff80000000;

SECTIONS
{
	/* linker trickery */
	. = kernel_physical_base + kernel_virtual_base;

	.text : AT(ADDR(.text) - kernel_virtual_base)
	{
		physical_start = ADDR(.text) - kernel_virtual_base;
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
		bss_start = .;
		*(COMMON)
		*(.bss .bss.*)	
	}
	physical_end = . - kernel_virtual_base;

	/DISCARD/ : { *(.comment) }
}
