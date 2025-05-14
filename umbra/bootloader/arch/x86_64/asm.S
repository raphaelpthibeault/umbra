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

	cli
	hlt

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

notification_string: .asciz "Begin asm.S... "

