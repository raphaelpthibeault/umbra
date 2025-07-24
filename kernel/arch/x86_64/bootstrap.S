/* bootstrap for the x86_64 kernel. Jumps to long mode and calls kmain */
	#include <memory_references.h>

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
	.align 8
multiboot2_tag_framebuffer:
	.word 5
	.word 0
	.long 20
	.long 0 # width = 0, let bootloader decide
	.long 0 # height = 0, let bootloader decide
	.long 0 # depth = 0, let bootloader decide

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
	# ENTRY MACHINE STATE:
	# EAX = magic value
	# EBX = protocol info
	movl %eax, V2P(multiboot_magic)
	movl %ebx, V2P(multiboot_addr)

	# setup stack
	movl V2P(stack_top), %esp

	# detect long mode support
	movl $0x80000000, %eax
	cpuid
	cmpl $0x80000001, %eax 
	jb panic32
	movl $0x80000001, %eax
	cpuid
	testl $(1 << 29), %edx
	jz panic32
	
	# setup page tables


	# enable paging


	loop:
	jmp loop

	# load gdt
	lgdtl V2P(gdt64_descriptor)
	.extern jump_to_long

	ljmp $0x08, $(V2P(jump_to_long))


	.align 4
panic32:
	jmp panic32

	.section .data

	.align 4
	.globl multiboot_magic
	.globl multiboot_addr

multiboot_magic:
	.long 0
multiboot_addr:
	.long 0

.globl gdt64_start
.globl gdt64_tss
.globl gdt64_descriptor

gdt64_start:
.null:
	.quad 0x0000000000000000
.kernel_code:
	# Kernel Code Segment (Ring 0)
  # Flags: Present, DPL 0, System, Execute-Only, 64-bit
	.quad 0x002098000000000
.kernel_data:
	# Kernel Data Segment (Ring 0)
	# Flags: Present, DPL 0, System, Read/Write
	.quad 0x0000920000000000
.user_code:
	# User Code Segment (Ring 3)
	# Flags: Present, DPL 3, System, Execute-Only, 64-bit
	.quad 0x0020f80000000000
.user_data:
	# User Data Segment (Ring 3)
	# Flags: Present, DPL 3, System, Read/Write
	.quad 0x0000f20000000000
gdt64_end:

gdt64_tss:
	# TSS Descriptor (16 bytes)
	.word 0						# Limit (low)
	.word 0						# Base (low)
	.byte 0						# Base (middle)
	.byte 0b10001001  # Access: Present, DPL 0, 64-bit TSS Available
	.byte 0b00000000  # Granularity/Flags
	.byte 0						# Base (high)
	.long 0						# Base (upper 32 bits)
	.long 0						# Reserved

gdt64_descriptor:
	.word gdt64_end - gdt64_start - 1 # 16-bit size (Limit)
	.quad gdt64_start # Base Address


	.section .bss

	.align 16
stack_bottom:
	.skip 16384 /* 16KiB */
	.globl stack_top
stack_top:

	.align 4096	
p4_table:	# PML4
	.skip 4096
p3_table: # PDPR
	.skip 4096
p3_table_hh: # PDPR
	.skip 4096
p2_table: # PDP
	.skip 4096
p1_table: 
	.skip 8192
	


