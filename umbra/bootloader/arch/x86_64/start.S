	.text 
	.code16

	.globl start
	/*
	 * start is loaded at 0x8000 and is jumped to with CS:IP 0:0x8000 from boot.S
	 * Recall, only 1 sector is loaded. We must load the rest of the sectors to perform stage2 
	 **/
start:
	movw $notification_string, %si
	call putstr


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

notification_string: .asciz "Starting stage2... "

/* should only be 1 sector */
.org 512
