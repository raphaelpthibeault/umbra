	.text 
	.code16

	.globl start
	/*
	 * start is loaded at 0x7c00 and is jumped to with CS:IP 0:0x7c00
	 **/
start:
	cli									/* UNSAFE NOW */
	/* I will assume that the BIOS will pass the boot drive correctly  */
	/* I will also assume that the BIOS will jump correctly to 0x0000:0x7C00 */

	/* init segments as offset from 0 */
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %ss

	/* setup stack */
	movw $0x2000, %sp

	sti									/* SAFE AGAIN */

	/* save drive reference:
   * I assume I do not need to force a disk reference to use a specific drive.
   * So I will save the space in this boot sector by not having a boot_drive allocated and simply push to the stack 
	 **/
	pushw %dx	
	
	/* print notificatin message to screen */	
	movw $notification_string, %si
	call putstr

	/* goal: the goal here is to load the first sector of stage 2, and then have stage2 figure the rest out
	 * location: 0x8000
	 * 
	 **/
	
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

notification_string:	.asciz "UMBRA BOOTLOADER "

/* BIOS magic number */
.org 510
	.byte 0x55
	.byte 0xAA
