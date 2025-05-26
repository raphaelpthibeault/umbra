OUTPUT_FORMAT("binary")
ENTRY(boot_menu)

SECTIONS
{
	/* */
	. = 0xf000;

	.text : 
	{
		PROVIDE(_stage3_addr = .);
		*(.text .text.*)
	}

	.rodata : 
	{
		*(.rodata .rodata.*)
	}
	
	.data : 
	{
		PROVIDE(_data_begin = .);
		*(.data .data.*)
		PROVIDE(_data_end = .);
	}

	.bss : 
	{
		PROVIDE(_bss_start = .);
		*(.bss .bss.*)
		PROVIDE(_bss_end = .);
	}
}

