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

multiboot2_header_end:

# stack

	.section .bootstrap
	.code32
	.align 4
start:
	jmp start
