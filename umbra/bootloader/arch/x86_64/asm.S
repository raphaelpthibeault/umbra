/* Collection of util assembly functions for core / stage2
 **/

	.text
	.code16 /* 16-bit functions */

/* putstr_bios:  write the string pointed to by %si
 * WARNING: trashes %si, %ax, and %bx
 *
 * Use BIOS "int 10H Function 0Eh" to write character in teletype mode
 *	%ah = 0xe	%al = character
 *	%bh = page	%bl = foreground color (graphics modes)
 **/
	 .globl putstr_bios
1: 
	movw $0x0001, %bx
	movb $0xe, %ah
	int $0x10						/* display byte */
putstr_bios:
	lodsb 
	cmpb $0, %al
	jne 1b							/* if not end of null, jump to display */
	ret

/* 
 * %si: number
 **/
puthex:
	pushw %ax
	pushw %bx
	pushw %cx
	pushw %dx

	movb $0xe, %ah
	movw $0x0001, %bx

	/* print prefix 0x */
	movb $'0', %al
	int $0x10
	movb $'x', %al
	int $0x10

	/* 4 nibbles left */
	movw $4, %cx

puthex_loop:
	cmpw $0, %cx
	je puthex_loop_end

	/* extract most significant nibble */
	movw %si, %dx
	shrw $12, %dx
	/* %dx >= 10 means 'A'-'F' */
	cmpw $10, %dx
	jge puthex_alpha
	/* numeric, nibble to ascii: '0' + nibble value */
	movb $'0', %al
	addb %dl, %al
	jmp puthex_gotchar

puthex_alpha:
	/* alpha, nibble to ascii: 'A' + nibble - 10 */
	subb $10, %dl
	movb $'A', %al
	addb %dl, %al

puthex_gotchar:
	int $0x10
	/* left shift to move next nibble to most significant pos */
	shlw $4, %si
	decw %cx
	jmp puthex_loop

puthex_loop_end:
	popw %dx
	popw %cx
	popw %bx
	popw %ax

	ret


	.code32 /* 32-bit functions */
/* no longer have BIOS-utilities, use VGA memory for printing */
	.text
	.globl clear_screen
clear_screen:
	pushal
		
	movl $0x000B8000, %ebx	/* VGA start */
	movl $0xfa0, %ecx				/* VGA size */
	movl $0x0, %edx					/* current offset */
	
clear_screen_loop:
	cmpl %ecx, %edx	
	jge clear_screen_loop_end /* offset >= size */
	
	pushl %edx
	
	movb $' ', %al
	movb $0x0f, %ah

	addl %ebx, %edx
	movw %ax, (%edx)

	popl %edx
	addl $2, %edx		/* move to next cell in VGA memory ; note each cell is 2 bytes */
	
	jmp clear_screen_loop

clear_screen_loop_end:
	popal
	ret
	
/* %esi: string */
	.globl putstr_pm
putstr_pm:
	pushal
	movl $0x000B8000, %edx

putsrpm_loop:
	cmpb $0, (%esi) 
	je putsrpm_loop_end

	movb (%esi), %al
	movb $0x0f, %ah

	movw %ax, (%edx)

	addl $1, %esi		/* move to next char in string */
	addl $2, %edx		/* move to next cell in VGA memory ; note each cell is 2 bytes */
	
	jmp putsrpm_loop

putsrpm_loop_end:
	popal
	ret

