	.section .rodata
invalid_idt:
	.quad 0

	.text
	.code32

	/* generic / common spinup called with function pointer to the protocol spinup e.g. multiboot1, multiboot2, etc */
	.globl spinup
spinup:
	#pushl %ebp
	#movl %esp, %ebp

	cli
	lidt invalid_idt

	# TODO flush irqs

	xorl %eax, %eax
	lldt %ax

	# don't need return address
	addl $4, %esp

	# get function address
	popl %edi

	# don't need argument count
	addl $4, %esp

	movl $0x00000011, %eax
	movl %eax, %cr0

	xorl %eax, %eax
	movl %eax, %cr4

	call *%edi

