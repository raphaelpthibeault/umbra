OUTPUT_FORMAT("binary")
ENTRY(stage3_start)

SECTIONS
{
	. = 0xf000;

	.text : 
	{
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

