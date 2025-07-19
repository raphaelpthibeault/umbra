/* bootstrap for the x86_64 kernel. Jumps to long mode and calls kmain */

	.section .multiboot
	.code32
.set MULTIBOOT2_MAGIC, 0xe85250d6
.set MULTIBOOT2_ARCH, 0
.set MULTIBOOT2_LENGTH, (multiboot2_header_end - multiboot2_header)
.set MULTIBOOT2_CHECKSUM, -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH + MULTIBOOT2_LENGTH)

	.align 8
multiboot2_header:
	.long MULTIBOOT2_MAGIC
	.long MULTIBOOT2_ARCH
	.long MULTIBOOT2_LENGTH
	.long MULTIBOOT2_CHECKSUM

/* the requests */
	/*
	.align 8
# address tag
multiboot2_tag_address:
	.word 2
	.word 0
	.long 24
	.long multiboot2_header # header address
	.long physical_start		# load address
	.long bss_start					# load end address
	.long physical_end			# bss end address

	.align 8
multiboot2_tag_entry_address:
	.word 3
	.word 0
	.long 12
	.long start_multiboot2
	*/

	.align 8
multiboot2_tag_framebuffer:
	.word 5
	.word 0
	.long 20
	.long 0 # width = 0, let bootloader decide
	.long 0 # height = 0, let bootloader decide
	.long 0 # depth = 0, let bootloader decide

	.align 8
multiboot2_tag_relocatable:
	.word 10
	.word 0
	.long 24
	.long 0x100000  # Start
	.long 0x1000000 # Maximum load address
	.long 4096      # Request page alignment
	.long 1         # Load at lowest available

# end tag
	.align 8
	.word 0
	.word 0
	.long 8

multiboot2_header_end:

	.section .bootstrap
	.code32
	.align 4
	.globl start
start:
	#movl %ebx, %ecx
	#addl $16, %ecx # use boot_drive to thunk eip
	movl %edi, %ebx
	movl %esi, %eax

	#popl %ecx
	#subl $forward, %ecx
	# TODO setup stack
	#movl $stack_top, %esp
	#addl %ecx, %esp
	# 'assert stack is 16-byte aligned
	#and $-16, %esp

	jmp jmp_to_long

	.align 4
jmp_to_long:
	jmp jmp_to_long


.section .bss
stack_bottom:
	.skip 16384 /* 16KiB */
	.global stack_top
stack_top:

