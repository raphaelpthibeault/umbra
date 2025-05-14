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

	/* setup first call to bootloop */


bootloop:


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
/* the sizeof(umbra_boot_blocklist) blocklist is 12 bytes 
 * 
 *	struct umbra_boot_blocklist {
 *		uint64_t start;
 *		uint16_t length;
 *		uint16_t segment;
 *	} __attribute__((packed));
*/
.org 512 - 12

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
