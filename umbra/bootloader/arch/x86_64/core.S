#include <asm.h>

	.text 
	.code16

	.globl start
	/*
	 * loaded at 0x8200
	 **/
start:
	pushw %si
	movw $notification_string, %si
	call putstr_bios
	popw %si

	cli								/* UNSAFE ZONE */

	/* real core of the bootloader now */
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %ss
	movw %ax, %es

	/* setup stack again, different stack than in boot.S and start.S */
	movl $STACK_OFF, %ebp
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
	
	DATA32 call real_to_prot
	.code32
	cld

	call clear_screen
	pushw %si
	movw $pm_mode_string, %si
	call putstr_pm
	popw %si

	movb boot_drive, %dl	
	pushl %edx

	.extern boot_main
	call boot_main

	# should never return	
	cli
	hlt

real_to_prot:
	.code16
	cli

	lgdtl gdtdesc

	/* turn on PM by setting bit 0 if control register 0, note that cr0 cannot be modified directly */
	movl %cr0, %eax
	orl $CR0_PE_ON, %eax
	movl %eax, %cr0

	/* far/long jump to relocation, flush prefetch queue and reload %cs */
	ljmpl $PROT_CSEG, $protcseg
	
protcseg:
	.code32
	/*  in 32-bit PM mode, now need to load DS, ES, FS, GS, SS, ESP
   * i.e. tell all segment registers to point to flat-mode data segment and then do stack stuff 
	 **/

	movw $PROT_DSEG, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	/* put the return address in a known safe location */
	movl (%esp), %eax
	movl %eax, STACK_OFF

	/* get protected-mode stack */
	movl protstack, %eax
	movl %eax, %esp
	movl %eax, %ebp

	/* get return address onto correct stack */ 
	movl STACK_OFF, %eax 
	movl %eax, (%esp)

	xorl %eax, %eax

	sidt realidt
	lidt protidt

	ret

	
	.data
notification_string: .asciz "Begin core ... "
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

realidt:
	.word 0x400
	.long 0

protidt:
	.word 0
	.long 0

