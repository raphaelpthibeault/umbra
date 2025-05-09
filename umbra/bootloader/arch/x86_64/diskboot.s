.set	VGA_TEXT_COLOR_80x25,0x03

.text

.code16
.globl start

start:
	cld			
	cli			

	/* setup stack */
	xorw	%ax,%ax
	movw	%ax,%ss		
	movw	%ax,%ds
	movw	%ax,%es
	sti

	/* set video mode to 16bit color text mode */
	movb	VGA_TEXT_COLOR_80x25,%al
	movb	$0x00,%ah
	int	$0x10

	movw	$msg_welcome,%si	/* %ds:(%si) -> welcome message */
	call	putstr

	cli
	hlt


/* display a null-terminated string */
putstr:
putstr.load:
	lodsb							/* load %al from %ds:(%si), then incl %si */
	testb	%al,%al			/* stop at null */
	jnz	putstr.putc		/* call the function to output %al */
	ret								/* return if null is reached */
putstr.putc:
	call	putc				/* output a character %al */
	jmp	putstr				/* go to next character */
putc:
	pushw	%bx					/* save %bx */
	movw	$0x7,%bx		/* %bh: page number for text mode */
										/* %bl: color code for graphics mode */
	movb	$0xe,%ah		/* BIOS: put char in tty mode */
	int	$0x10					/* call BIOS, print a character in %al */
	popw	%bx					/* restore %bx */
	ret

msg_welcome:
	.ascii	"--- umbra bootloader ---\r\n\n"

/* BIOS magic number */
.org 510
	.byte 0x55
	.byte 0xAA


