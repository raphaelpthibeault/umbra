
	.extern kmain

	.extern init_start
	.extern init_text_end

	.extern init_data_start
	.extern init_data_end

	.extern text_start
	.extern text_end

	.extern data_start
	.extern kernel_end

	#include <memory_references.h>

.macro map_pages from to flags
	movl $((\to - \from) >> 12), %ecx
	movl $(\from | \flags), %eax
	movl $( (V2P(pt_table)) + (\from >> 9) ), %edi
1:
	stosl
	addl $0x1000, %eax
	movl $0, (%edi)
	addl $4, %edi
	loop 1b
.endm

.macro map_pages2 from to flags
	movl $\to, %ecx
	subl $\from, %ecx
	shrl $12, %ecx
	movl $\from, %eax
	movl %eax, %edi
	orl $\flags, %eax
	shrl $9, %edi
	addl $(V2P(pt_table)), %edi
1:
	stosl
	addl $0x1000, %eax
	movl $0, (%edi)
	addl $4, %edi
	loop 1b
.endm

	.section .bss

	.align 16
stack_bottom:
	.skip 16384 /* 16KiB */
	.globl stack_top
stack_top:

	.align 4096	
	.globl GDT_table
GDT_table:
	.skip 4096
	.globl IDT_table
IDT_table:
	.skip 4096
pml4_table:
	.skip 4096
pdp_table:
	.skip 4096
pd_table:
	.skip 4096
pt_table:
	.skip 4096


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

	.section .init.text
	.code32
	.align 4
	.globl start
	.type start, @function
start:
	# ENTRY MACHINE STATE:
	# EAX = magic value
	# EBX = protocol info

	cld
	cli

	# KEEP USING BOOTLOADER's STACK UNTIL LONG_MODE

	pushl %eax # magic value
	pushl %ebx # protocol info

	# clear gdt + idt
	xorl %eax, %eax
	movl $(V2P(GDT_table)), %edi
	movl $(GDT_LENGTH), %ecx
	rep stosb
	xorl %eax, %eax
	movl $(V2P(IDT_table)), %edi
	movl $(IDT_LENGTH), %ecx
	rep stosb

	# setup barebones GDT
	movl $(V2P(gdt_start)), %esi
	movl $(V2P(GDT_table)), %edi
	movl $(10 * 8), %ecx
	rep movsb

	# detect long mode support
	movl $0x80000000, %eax	# extended-function 80000000h
	cpuid										# is largest extended function
	cmpl $0x80000000, %eax	# check for any function > 80000000h
	jbe panic32							# if not, no long mode.
	movl $0x80000001, %eax	# extended-function 80000001h
	cpuid										# now EDX = extended-features flags
	btl $29, %edx						# test if long mode is supported
	jnc panic32							# exit if not supported.
	
	# setup page tables
	# for sanity, zero page tables
	xorl %eax, %eax
	movl $(V2P(pml4_table)), %edi
	movl $0x4000, %ecx
	rep stosb

	# chain the tables together
	# PML4[0] = &PDP[0] | (PRESENT, WRITABLE)
	movl $(V2P(pdp_table) + 0x03), (V2P(pml4_table))
	# PML4[511] = &PDP[0] | (PRESENT, WRITABLE)
	movl $(V2P(pdp_table) + 0x03), (V2P(pml4_table) + 0xff8)
	# PDP[0] = &PD[0] | (PRESENT, WRITABLE)
	movl $(V2P(pd_table) + 0x03), (V2P(pdp_table))
	# PDP[510] = &PD[0] | (PRESENT, WRITABLE)
	movl $(V2P(pd_table) + 0x03), (V2P(pdp_table) + 0xff0)
	# PD[0] = &PT[0] | (PRESENT, WRITABLE)
	movl $(V2P(pt_table) + 0x03), (V2P(pd_table))

	# black magic i.e. recursive mapping that mirrors the tables into virtual memory
	# PML4[510] = &PML4[0] | (PRESENT, WRITABLE)
	movl $(V2P(pml4_table) + 0x03), (V2P(pml4_table) + 0xff0)

	# map IVT and BDA as read-only
	map_pages 0x00000000, 0x00001000, 0x01
	map_pages 0x0009f000, 0x000a0000, 0x01

	# map VGA buffer as read-write (32 entries * 4kB = 128kB)
	map_pages 0x000a0000, 0x000c0000, 0x03

	# map BIOS as read-only (32 entries * 4kB = 128kB)
	map_pages 0x000e0000, 0x00100000, 0x01

	# map text part of bootstrap section as read-only
	map_pages2 V2P(init_start), V2P(init_text_end), 0x01
	# map data part of bootstrap section as read0write
	map_pages2 V2P(init_data_start), V2P(init_data_end), 0x03

	# map text part of kernel as read-only
	map_pages2 V2P(text_start), V2P(text_end), 0x01
	# map read-only data part of kernel as read-only
	map_pages2 V2P(rodata_start), V2P(rodata_end), 0x01
	# map data part of kernel as read-write
	map_pages2 V2P(data_start), V2P(kernel_end), 0x03

	# enable paging
	# enable pae + pse + pge
	movl %cr4, %eax
	orl $0xb0, %eax
	movl %eax, %cr4

	# set page dir base
	movl $(V2P(pml4_table)), %eax
	movl %eax, %cr3

	#	set EFER (MSR number 0xc0000080) to enable syscall/sysret: bit 0, and LMEL: bit 8
	movl $0xc0000080, %ecx
	rdmsr
	orl $0x00000101, %eax
	wrmsr

	popl %ebx # protocol info
	popl %eax # magic value

	# enable paging (activate long mode), enable floating point exception
	movl %cr0, %ecx
	orl $0x8000002a, %ecx
	movl %ecx, %cr0

	# load gdt leave parentheses around addr for memory indirection
	lgdt (V2P(gdtr32))

	ljmpl $(KERNEL_CODE), $(V2P(long_mode))

panic32:
	jmp panic32

	.section .text
	.code64
	.align 8

	.type long_mode, @function
long_mode:
	# setup stack
	movabsq $stack_top, %rsp

	# black magic
	movabsq $1f, %rdx
	jmpq *%rdx

1:
	# put multiboot2 info into the correct registers
	xorq %rdi, %rdi
	xorq %rsi, %rsi

	movl %eax, %edi # magic value
	movl %ebx, %esi # protocol info

	# now need to load descriptor tables and set segment registers
	# reload descriptor tables
	movabsq	$(gdtr64), %rax
	lgdt (%rax)
	movabsq	$(idtr64), %rax
	lidt (%rax)
	movabsq	$(ldtr64), %rax
	lldt (%rax)

	# set 64-bit fs and gs offset to 0
	xorl %eax, %eax
	movl %eax, %edx
	movl $0xc0000100, %ecx
	wrmsr
	incq %rcx
	wrmsr

	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	# clear identity mapping, preserve only the kernel 
	movabsq $pml4_table, %rax
	movl $0x0, (%rax)
	movabsq $pdp_table, %rax
	movl $0x0, (%rax)

	# go to kernel
	movabsq $(kmain), %rax
	callq *%rax

	# do some stuff when if it returns? 
panic64:
	jmp panic64

	.section .init.text
	.code32
	
	.align 8
	gdtr32:
		.word GDT_LENGTH
		.long V2P(GDT_table)

	.globl gdtr64
	gdtr64:
		.word GDT_LENGTH
		.quad GDT_table
	
	.globl idtr64
	idtr64:
		.word IDT_LENGTH
		.quad IDT_table
	
	ldtr64: # no LDT, load with zero
		.word 0
		.quad 0

	.section .init.data
	.align 8

gdt_start:
	# 0x00 = Null descriptor
	.long 0
	.long 0

	# 0x08 = 32bit data segment
	.long 0x0000ffff
	.long 0x00cf9200

	# 0x10 = 32bit code segment
	.long 0x0000ffff
	.long 0x00cf9A00

	# 0x18 = 32bit stack segment
	.long 0x0000ffff
	.long 0x00cf9200

	# 0x20 = kernel code segment
	.long 0x0000ffff
 #.long 0x002f9a00
	.long 0x00af9a00

	# 0x28 = kernel stack segment
	.long 0x0000ffff
	.long 0x00cf9200

	# 0x30 = kernel data segment
	.long 0x0000ffff
	.long 0x00cf9200

	# 0x38 = user data segment
	.long 0x0000ffff
	.long 0x00cff200

	# 0x40 = user stack segment
	.long 0x0000ffff
	.long 0x00cff200

	# 0x48 = user code segment
	.long 0x0000ffff
	.long 0x002ffa00
gdt_end:

gdt_tss:
	# TSS Descriptor (16 bytes)
	.word 0						# Limit (low)
	.word 0						# Base (low)
	.byte 0						# Base (middle)
	.byte 0b10001001  # Access: Present, DPL 0, 64-bit TSS Available
	.byte 0b00000000  # Granularity/Flags
	.byte 0						# Base (high)
	.long 0						# Base (upper 32 bits)
	.long 0						# Reserved


