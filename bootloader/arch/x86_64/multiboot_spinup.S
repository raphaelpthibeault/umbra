	.text
	.code32

	.globl multiboot_spinup
multiboot_spinup:
	subl $6, %esp
	movw $0x3ff, -2(%esp)
	movl $0, (%esp)
	lidt -2(%esp)
	addl $6, %esp

	addl $4, %esp # return address

	popl %ebx # reloc_stub
	popl %esi # magic
	popl %edi # protocol_info
	popl %ecx # entry_point
	popl %eax # elf_ranges
	popl %edx # elf_ranges_count

	jmp *%ebx
