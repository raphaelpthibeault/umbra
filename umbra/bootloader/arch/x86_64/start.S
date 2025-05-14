	.text 
	.code16

	.globl start
	/*
	 * start is loaded at 0x8000 and is jumped to with CS:IP 0:0x8000 from boot.S
	 * Recall, only 1 sector is loaded. We must load the rest of the sectors to perform stage2 
	 **/
start:
	/* continue the stack of boot.img, I assume some registers are set to some values that should be saved */
	
	/* save drive reference */
	pushw %dx	

	pushw %si
	movw $notification_string, %si
	call putstr
	popw %si

	/*  did length actually get updated by mkimage? */
	movw $blocklist_length, %ax
	testw %ax, %ax
	jz panic
	
	/* setup first call to bootloop */
	movw $firstlist, %di

	/* save the start of sectors to load in %ebp ( sector nb2 - recall sector numbering starts at 0) */
	movl (%di), %ebp

bootloop:
	/* check number of sectors to read */
	cmpw $0, 8(%di) 

	/ * if zero, jump to end */
	je bootit

setup_sectors:
	/* check if lba or chs mode */	
	cmpb $0, -1(%si)
	je panic /* do not care to support chs mode */
	
	/* load local sector start */
	movl (%di), %ebx		/* first long */
	movl 4(%di), %ecx		/* second long */

	/* some Pheonix BIOSes are limited to 0x7f = 127 sectors */
	xorl %eax, %eax
	movb $0x7f, %al

	cmpw %ax, 8(%di)
	jg 1f
	/* if 127 is less, set num to nbsectors */
	movw 8(%di), %ax	

	/* limitted to reading 127 sectors at a time */
1: 
	/* subtract from nbsectors so we have 0 sectors left to read after, or nbsectors-127 left to read */
	subw %ax, 8(%di)
	
	/* add to logical sector start */
	addl %eax, (%di)
	adcl $0, 4(%di)			/* add with carry */

	/* now we setup the disk address packet from boot.S */
	/* size and reserved byte */	
	movw $0x0010, (%si)
	/* number of sectors */
	movw %ax, 2(%si)
	/* absolute address */
	movl %ebx, 8(%si)
	movl %ecx, 12(%si)

	/* segment of buffer address */
	movw $0x7000, 6(%si)

	/* offset of buffer address */
	movw $0, 4(%si)
	
	/* save %ax from interrupt call clobber*/
	pushw %ax
	
	/* BIOS interrupt call INT 0x13 AH=42 reads to memory
	 * %ah=0x42, %dl=drive number, %ds:%si=segment:offset of disk address packet
	 **/
	
	movb $0x42, %ah
	int $0x13

	jc panic /* eventually I will have to add actual error messages */
	
	movw $0x7000, %bx

copy_buffer:
	/* load addresses for copy from disk buffer to destination */
	
	movw 10(%di), %es /* load destination segment */
	/* restore %ax from before BIOS interrupt call */
	popw %ax
	
	/* determine next possible destination address assuming 512-byte sectors */
	shlw $5, %ax			/* shift left %ax 5 bits */
	addw %ax, 10(%di) /* add corrected value to the destination address for next time */

	/* save addressing registers */
	pusha
	pushw %ds

	/* get copy length */
	shlw $3, %ax
	movw %ax, %cx

	xorw %di, %di		/* zero offset of destination addresses */
	xorw %si, %si		/* zero offset of source addresses */
	movw %bx, %ds		/* restore the source segment */

	cld /* set copy direction to forward */

	/* do copy */
	rep			/* set repeat */
	movsw		/* mov string word */

	/* pop addressing registers */
	popw %ds
	popa

	/* check if done with this dataset*/
	cmpw $0, 8(%di)
	jne setup_sectors

	/* update position to load from */
	/* may have more blocklists (sizeof 12), recall the list grows upwards from firstlist */
	subw $12, %di
	/* jump back to bootloop */
	jmp bootloop


bootit:
	/* pop drive reference from beginning of start */
	popw %dx

	movw $done_string, %si
	call putstr

	/* jump to core */
	ljmp	$0, $0x8200

panic:
	movw $panic_string, %si
	call putstr

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
panic_string: .asciz " PANIC!"
done_string: .asciz " Jumping to core... "

/* should only be 1 sector */
/* the sizeof(umbra_boot_blocklist) blocklist is 12 bytes 
 * 
 *	struct umbra_boot_blocklist {
 *		uint64_t start;
 *		uint16_t length;
 *		uint16_t segment;
 *	} __attribute__((packed));
*/

/* do I have to be sure to add a zero blocklist in mkimage or do I just allocate some .word-s?*/
/* have firstlist - 12, so firstlist - 12 + 8 = firstlist - 4 so some .word should be fine */

.word 0 /* firstlist - 4 */
.word 0

.org 512 - 12

/* blocklists grow upwards from here */
firstlist:
blocklist_start:
	/* sector start parameter, in logical sectors from start of disk (sector 0) */
	.long 2, 0 
blocklist_length:
	/* number of sectors to read */
	.word 0
blocklist_segment:
	/* segment of the starting address to load the data into */
	.word (0x800 + 0x20)
