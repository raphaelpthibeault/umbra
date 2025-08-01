OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start)

kernel_physical_base = 1M;
kernel_virtual_base = 0xffffffff80000000;
kernel_start = kernel_physical_base + kernel_virtual_base;
init_start = kernel_start;

PHDRS
{
	init PT_LOAD FILEHDR;
	user PT_LOAD;
	kernel PT_LOAD;
	dynamic PT_DYNAMIC;
}

SECTIONS
{
	/* linker trickery */
	. = kernel_start;

	.init.text (init_start + SIZEOF_HEADERS) : AT(ADDR(.init.text) - kernel_virtual_base)
	{
		. = ALIGN(16);
		*(.multiboot)
		
		init_text_start = .;
		*(.init.text)
		. = ALIGN(4K);
		init_text_end = .;
	} :init
		
	.init.data : AT(ADDR(.init.data) - kernel_virtual_base)
	{
		init_data_start = .;
		*(.init.data)
		. = ALIGN(4K);
		init_data_end = .;
	} :init


	text_start = .;
	.text : AT(ADDR(.text) - kernel_virtual_base)
	{
		*(.text .text.*)
	}
	. = ALIGN(4K);
	text_end = .;


	rodata_start = .;
	.rodata : AT(ADDR(.rodata) - kernel_virtual_base)
	{
		*(.rodata .rodata.*)	
	}
	. = ALIGN(4K);
	rodata_end = .;

	. = ALIGN(4K);
	data_start = .;
	.data : AT(ADDR(.data) - kernel_virtual_base)
	{
		*(.data .data.*)	
	}
	
	. = ALIGN(4K);
	.bss : AT(ADDR(.bss) - kernel_virtual_base)
	{
		bss_start = .;
		*(COMMON)
		*(.bss .bss.*)	
		*(.stack)
	}
	. = ALIGN(4K);

	kernel_end = .;

	/DISCARD/ : { *(.comment) }
}
