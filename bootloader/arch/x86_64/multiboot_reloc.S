	.data
	.code32
	.globl multiboot_reloc_stub
multiboot_reloc_stub:
	jmp .code

	.balign 4, 0

	# EBX = self
	# ESI = magic value
	# EDI = protocol info
	# ECX = entry point
	# EAX = ranges
	# EDX = ranges count

.code:
	# setup temp stack
	movl %ebp, %esp
	addl $(.temp_stack_top - multiboot_reloc_stub), %esp

	pushl %edi
	pushl %esi
	pushl %ecx

.ranges_loop:
	testl %edx, %edx
	jz .ranges_loop_end

	# load relocation params from the current range
	movl (%eax), %esi		# ESI = range.relocation
	movl 8(%eax), %edi	# EDI = range.target
	movl 16(%eax), %ecx # ECX = range.length

	# copy mem range
	rep movsb

	# sizeof(struct relocation_range) == 24
	addl $24, %eax

	decl %edx
	jmp .ranges_loop

.ranges_loop_end:
	# done relocating
	# restore registers for kernel entry
	popl %ecx
	popl %esi
	popl %edi

	pushl %ecx

	# per Multiboot2 spec (machine state section)
	movl %esi, %eax # EAX = magic value
	movl %edi, %ebx # EBX = protocol info
	# as a courtesy to the kernel
	xorl %ecx, %ecx
	xorl %edx, %edx
	xorl %esi, %esi
	xorl %edi, %edi
	xorl %ebp, %ebp

	ret

	.align 16
.temp_stack:
	.quad 0, 0, 0
.temp_stack_top:

	.globl multiboot_reloc_stub_end
multiboot_reloc_stub_end:

.section .note.GNU-stack, "", @progbits
