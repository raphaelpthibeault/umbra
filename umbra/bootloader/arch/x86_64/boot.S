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
	 * buffer: 0x7000
	 **/

	/* set %si to the disk address packet */
	movw $disk_address_packet, %si

	/* check if LBA is supported */
	movb $0x41, %ah
	movw $0x55aa, %bx
	int $0x13

	/* %dl may have been clobbered by INT 13, AH=41H */
	popw %dx
	pushw %dx

	/* do not care to support CHS, so if LBA is not supported just panic */
	jc panic
	cmpw $0xaa55, %bx
	jne panic
	andw $1, %cx
	jz panic
	
lba_mode:
	xorw %ax, %ax
	movw %ax, 4(%si)

	incw %ax
	/* set the mode to non-zero */
	movb %al, -1(%si)

	/* the blocks */
	movw %ax, 2(%si)

	/* the size and the reserved byte */
	movw $0x0010, (%si)

	/* the absolute address */
	movl stage2_sector, %ebx
	movl %ebx, 8(%si)
	movl stage2_sector_high, %ebx
	movl %ebx, 12(%si)

	/* the segment of buffer address */
	movw $0x7000, 6(%si)

	movb $0x42, %ah
	int $0x13

	jc error

	movw $0x7000, %bx
	
copy_buffer:
	pusha
	pushw %ds

	movw $0x100, %cx
	movw %bx, %ds
	xorw %si, %si
	movw $0x8000, %di
	movw %si, %es

	cld

	rep
	movsw

	popw %ds
	popa

	/* jump to stage2 */
	/* jmp *(0x8000) */

	/* print notification message to screen */	
	movw $notification_string, %si
	call putstr

panic:
	/* print panic message to screen */	
	movw $panic_string, %si
	call putstr
	cli
	hlt

error:
	/* print error message to screen */	
	movw $error_string, %si
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

notification_string: .asciz "UMBRA BOOTLOADER "
panic_string: .asciz " PANIC "
error_string: .asciz " ERROR "

mode:
	.byte 0
disk_address_packet:
sectors:
	.long	0
heads:
	.long	0
cylinders:
	.word	0
sector_start:
	.byte	0
head_start:
	.byte	0
cylinder_start:
	.word	0

stage2_sector:
	.long 1
stage2_sector_high:
	.long 0


/* BIOS magic number */
.org 510
	.byte 0x55
	.byte 0xAA
