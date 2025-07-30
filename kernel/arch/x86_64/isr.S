	#include <memory_references.h>

# push all general-purpose registers (quad)
.macro pushagrq
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rsi
	pushq %rdi
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
.endm

# pop all general-purpose registers (quad)
.macro popagrq
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rdi
	popq %rsi
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax
.endm

# push all control regisers (quad)
.macro pushacrq
	movq %cr0, %rax
	pushq %rax
	movq %cr2, %rax
	pushq %rax
	movq %cr3, %rax
	pushq %rax
	movq %cr4, %rax
	pushq %rax
.endm

# pop all control registers (quad)
.macro popacrq
	popq %rax
	movq %rax, %cr4
	popq %rax
	movq %rax, %cr3
	popq %rax
	movq %rax, %cr2
	popq %rax
	movq %rax, %cr0
.endm

.macro isr_stub number
isr\number:
	# check to see if this is an exception that pushes can error code
	.if \number == 8 || \number == 10 || \number == 11 || \number == 12 || \number == 13 || \number == 14 || \number == 17 || \number == 30
		pushq $\number
	.else
		pushq $0		
		pushq $\number
	.endif

	pushq %r15
	leaq isr_xframe_assembler(%rip), %r15
	jmp *%r15
.endm

# create and insert CPU exception isr into isr_stub_table
.altmacro
.macro isr_insert number
.align 16
	# create the stub that calls the isr
	.section .text
	isr_stub \number

	# allocate memory to store address of the isr# stub (aka insert function pointer in isr_stub_table)
	.section .data
	.quad isr\number
.endm
.noaltmacro

.extern __routine_handlers

.macro irq_stub number
irq\number:
	pushq %rbp
	pushq %r15
	pushq %rax
	xorl %eax, %eax
	movw %ds, %ax # <---
	cmpw $KERNEL_DATA, %ax 
	#jne .Lyes_task
	jne 1f
	# leaq .Lno_task(%rip), %r15
	leaq 2f(%rip), %r15
	jmp *%r15

1: # .Lyes_task: # not a problem for now
	
2: # .Lno_task: # interrupting kernel
	popq %rax
	popq %r15
	pushagrq
	leaq __routine_handlers(%rip), %r15
	movq \number * 8(%r15), %r15 # address in isr_stub_table - sizeof(uint64_t) == 8
	call *%r15
	popagrq
	popq %rbp
	iretq
.endm

# create and insert irq handler stub into isr_stub_table
.altmacro
.macro irq_insert number
.align 16
	.section .text
	irq_stub \number

	.section .data
	.quad irq\number
.endm
.noaltmacro

	.section .text
	.extern isr_exception_handler

isr_xframe_assembler:
	popq %r15
	pushq %rbp
	movq %rsp, %rbp
	pushagrq
	pushacrq
	movw %ds, %ax # <---
	pushq %rax
	pushq $0
	pushq %r15
	pushq %rax
	xorl %eax, %eax
	movw %ds, %ax # <---
	cmpw $KERNEL_DATA, %ax 
	#jne .Lyes_task
	jne 1f
	#leaq .Lno_task(%rip), %r15
	leaq 2f(%rip), %r15
	jmp *%r15
	
1: # .Lyes_task: # not a problem for now

2: # .Lno_task:
	# come from kernel mode, call the handler directly
	leaq 0x10(%rsp), %rdi
	leaq isr_exception_handler(%rip), %r15
	call *%r15

	# restore segment registers
	popq %rax
	popq %rax
	movw %ax, %ds
	movw %ax, %es

	# restore all registers and return from interrupt
	popacrq
	popagrq
	movw %ax, %ds
	movw %ax, %es
	popq %rbp
	addq $0x10, %rsp # clean up error code and int number from stack
	iretq

	.altmacro
.globl isr_stub_table
isr_stub_table:
	# CPU exceptions, 0-31
	.set i, 0
	.rept 32
		isr_insert %i
		.set i, i+1
	.endr
	# IRQ handler stubs 32-38
	.set i, 32 # i should be 32, but for sanity
	.rept 7
		irq_insert %i
		.set i, i+1
	.endr
	.noaltmacro
