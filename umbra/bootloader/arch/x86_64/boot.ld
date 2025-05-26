OUTPUT_FORMAT("binary")
ENTRY(start)

SECTIONS
{
	. = 0x7C00;

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
		*(.data .data.*)
	}

	.bss : 
	{
		*(.bss .bss.*)
	}
}

