OUTPUT_FORMAT("binary")
ENTRY(start)

SECTIONS
{
	. = 0x8200;

	.text : 
	{
		*(.text)
	}

	.rodata : 
	{
		*(.rodata)
	}
	
	.data : 
	{
		*(.data)
	}

	.bss : 
	{
		PROVIDE(_bss_start = .);
		*(.bss)
		PROVIDE(_bss_end = .);
	}
}

