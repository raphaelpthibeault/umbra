	/* 
	 * signature:	
	 *	void rm_int(uint8_t intnb, struct int_regs *regs)
	 *		__attribute__((regparm(3)));
	 * the attribute meaning the first 3 function parameters will in eax, edx, and ecx (respectively) instead of on the stack
	 *  (assuming they are of intergral type, which pointers are ; yes, in this case there are only 2 function parameters - use 3 anyways, apparently)
	 *	regs serves as both an in and out variable 
	**/
	.globl rm_int
rm_int:
	.code32
	pushf
	cli
	popf
	pushl    %ebp
	pushl    %ecx
	pushl    %eax
	pushl    %ebx
	pushl    %esi
	pushl    %edi	
	pushl    %edx

	# get function parameters
	movb %al, intnb 

	# "input" registers
	movl (%edx), %eax 
	movl %eax, bios_register_eax
	movw 4(%edx), %ax
	movw %ax, bios_register_es
	movw 6(%edx), %ax
	movw %ax, bios_register_ds
	movw 8(%edx), %ax
	movw %ax, bios_register_flags

	movl 12(%edx), %ebx
	movl 16(%edx), %ecx
	movl 20(%edx), %edi
	movl 24(%edx), %esi
	movl 28(%edx), %edx

	call prot_to_real
	.code16
	pushf
	cli
	mov %ds, %ax
	push %ax

	/* movw imm16, %ax*/
	.byte	0xb8
bios_register_es:
	.short 0
	movw %ax, %es
	/* movw imm16, %ax*/
	.byte	0xb8
bios_register_ds:
	.short 0
	movw %ax, %ds

	/* movw imm16, %ax*/
	.byte	0xb8
bios_register_flags:
	.short 0
	push %ax
	popf
	
	/* movl imm32, %eax*/
	.byte 0x66, 0xb8
bios_register_eax:
	.long 0
	
	/* int imm8.  */
	.byte 0xcd
intnb:	
	.byte 0

	movl %eax, %cs:bios_register_eax
	movw %ds, %ax
	movw %ax, %cs:bios_register_ds
	pop %ax
	mov %ax, %ds
	pushf
	pop %ax
	movw %ax, bios_register_flags
	mov %es, %ax
	movw %ax, bios_register_es

	popf

	call real_to_prot
	.code32

	popl %eax

	movl %ebx, 12(%eax)
	movl %ecx, 16(%eax)
	movl %edi, 20(%eax)
	movl %esi, 24(%eax)
	movl %edx, 28(%eax)

	movl %eax, %edx

	movl bios_register_eax, %eax
	movl %eax, (%edx)
	movw bios_register_es, %ax
	movw %ax, 4(%edx)
	movw bios_register_ds, %ax
	movw %ax, 6(%edx)
	movw bios_register_flags, %ax
	movw %ax, 8(%edx)

	popl %edi
	popl %esi
	popl %ebx
	popl %eax
	popl %ecx
	popl %ebp
	ret
