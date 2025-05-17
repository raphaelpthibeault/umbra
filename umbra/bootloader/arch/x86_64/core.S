	.text 
	.code16

	.globl start
	/*
	 * loaded at 0x8200
	 **/
start:
	pushw %si
	movw $notification_string, %si
	call putstr
	popw %si

	cli								/* UNSAFE ZONE */

	/* real core of the bootloader now */
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %ss
	movw %ax, %es

	/* setup stack again, different stack than in boot.S and start.S */
	/* 0x2000 - 0x10 */
	movl $0x1ff0, %ebp
	movl %ebp, %esp
	cli								/* SAFE AGAIN */
	
	/* save boot drive */
	movb %dl, boot_drive
	/* reset disk system, %ah = 0 */
	int $0x13

	/* enable A20 line */
	in $0x92, %al
	or $2, %al
	out %al, $0x92
	
	jmp real_to_prot
	cli
	hlt

prot_mode:
	.code32
	cld

	call clear_screen
	pushw %si
	movw $pm_mode_string, %si
	call putstrpm
	popw %si

	/* TODO: jump to bootloader C code 
	 * that will load the kernel from file system and jump to it
	 **/


	#.extern boot_main
	#call boot_main
	
	cli
	hlt


real_to_prot:
	.code16
	cli

	lgdtl gdtdesc

	/* turn on PM by setting bit 0 if control register 0, note that cr0 cannot be modified directly */
	movl %cr0, %eax
	orl $0x1, %eax
	movl %eax, %cr0

	/* far/long jump to relocation, flush prefetch queue and reload %cs 
	 * code segment: 0x08
	 * data segment: 0x10
	 **/
	ljmpl $0x08, $protcseg
	
protcseg:
	.code32
	/*  in 32-bit PM mode, now need to load DS, ES, FS, GS, SS, ESP
   * i.e. tell all segment registers to point to flat-mode data segment and then do stack stuff 
	 **/

	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	/* put the return address in a known safe location */
	movl (%esp), %eax
	movl %eax, 0x1ff0

	/* get protected-mode stack */
	movl protstack, %eax
	movl %eax, %esp
	movl %eax, %ebp

	/* get return address onto correct stack */ 
	movl 0x1ff0, %eax 
	movl %eax, (%esp)

	xorl %eax, %eax

	jmp prot_mode
	

	.code16 /* 16-bit functions */
/* putstr:  write the string pointed to by %si
 * WARNING: trashes %si, %ax, and %bx
 *
 * Use BIOS "int 10H Function 0Eh" to write character in teletype mode
 *	%ah = 0xe	%al = character
 *	%bh = page	%bl = foreground color (graphics modes)
 **/
1: 
	movw $0x0001, %bx
	movb $0xe, %ah
	int $0x10						/* display byte */
putstr:
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


	.data
notification_string: .asciz "Begin asm.S... "
pm_mode_string: .asciz "We are now in protected mode... "

boot_drive:
	.byte 0x00

protstack:
	.long 0x7fff0		

	.p2align 2 /* force 4-byte alignment */
gdt:
	.word 0, 0
	.byte 0, 0, 0, 0
	
	/* code segment */
	.word	0xFFFF, 0
	.byte	0, 0x9A, 0xCF, 0

	/* data segment */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0xCF, 0

	/* 16 bit real mode CS */
	.word	0xFFFF, 0
	.byte	0, 0x9E, 0, 0

	/* 16 bit real mode DS */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0, 0
	
gdtdesc:
	.word 0x27	/* limit = 39 ; */
	.long gdt		/* addr ; of start */

	.code32 /* 32-bit functions */
/* no longer have BIOS-utilities, use VGA memory for printing */
	.text
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
putstrpm:
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

