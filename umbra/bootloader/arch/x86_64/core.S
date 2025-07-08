#include <asm.h>

	.section .realmode
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
	sti								/* SAFE AGAIN */
	
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
	/*
	pushw %si
	movw $pm_mode_string, %si
	call putstr_pm
	popw %si
	*/

	/* TODO check presence of PCI, CPUID, MSRs 
	 * I am just assuming they are there for now
	 **/

	/* check for SSE: CPUID.01h:EDX.SSE[bit 25] needs to be set */
	movl $0x1, %eax
	cpuid
	testl $0x2000000, %edx
	jz panic

	/* enable SSE */
	movl %cr0, %eax
	andw $0xFFFB, %ax		# clear coprocessor emulation CR0.EM (bit 2)
	orw $0x2, %ax				# set coprocessor monitoring CR0.MP (bit 1)
	movl %eax, %cr0
	movl %cr4, %eax
	orw  $(3 << 9), %ax	# set CR4.OSFXSR (bit 9) and CR4.OSXMMEXCPT (bit 10)
											# (3 << 9) = (1 << 9) | (1 << 10) = 0x200 | 0x400 = 0x600
	movl %eax, %cr4


	movb boot_drive, %dl	
	pushl %edx

	.extern boot_main
	call boot_main

	# should never return	
	cli
	hlt


	.globl real_to_prot
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

	/* put return address onto correct stack */ 
	movl STACK_OFF, %eax 
	movl %eax, (%esp)

	xorl %eax, %eax

	sidt realidt
	lidt protidt

	ret

	.globl prot_to_real
prot_to_real:
	.code32
	lgdt gdtdesc	# just in case, for sanity
	
	sidt protidt
	lidt realidt

	/* save protected mode stack */
	movl %esp, %eax
	movl %eax, protstack

	/* get return address */
	movl (%esp), %eax
	movl %eax, STACK_OFF

	/* setup real mode stack */
	movl $STACK_OFF, %eax
	movl %eax, %esp
	movl %eax, %ebp

	/* setup segment lists */
	movw $PSEUDO_REAL_DSEG, %ax	
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	ljmp $PSEUDO_REAL_CSEG, $tmpcseg

tmpcseg:
	.code16
	/* clear PE bit of cr0 */
	movl %cr0, %eax
	andl $(~CR0_PE_ON), %eax
	movl %eax, %cr0

	/* flush prefetch queue, reload %cs */
	ljmpl	$0, $realcseg

realcseg:
	/* actually in real mode */
	xorl %eax, %eax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	sti
	ret

panic:
	jmp panic

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

	.code16
	.globl realidt
realidt:
	.word 0x400
	.long 0

	.code32
	.globl protidt
protidt:
	.word 0
	.long 0

