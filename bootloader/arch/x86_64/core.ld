OUTPUT_FORMAT("binary")
ENTRY(start)

SECTIONS
{
	. = 0x8200;

	.text : 
	{
		*(.realmode)
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

	/* it's very important that the bss is at the end of core.ld, specifically _bss_end */
	.bss : 
	{
		PROVIDE(_bss_start = .);
		*(.bss)
		PROVIDE(_bss_end = .);
	}
}

