/* bootstrap for the x86_64 kernel. Jumps to long mode and calls kmain */

	.section .multiboot
	.code32




	.section .bootstrap
	.code32
	.align 4
start:
	jmp start
