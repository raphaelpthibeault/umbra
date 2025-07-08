	.section .realmode
	.extern except

/* generate handlers for exceptions 0-31 */

.macro irq_stub number
irq\number:
	# check to see if this is an exception that pushes can error code
	.if \number == 8 || \number == 10 || \number == 11 || \number == 12 || \number == 13 || \number == 14 || \number == 17 || \number == 30
		popl %eax
	.else
		xorl %eax, %eax
	.endif

	pushl %ebp
	movl %esp, %ebp
	pushl %eax
	pushl $\number
	call except
.endm

.altmacro
.macro irq_insert number
.align 16
	# create the stub that calls the isr handler, except
	.section .text
	irq_stub \number

	# allocate memory to store address of the irq# stub (aka insert function pointer in isr_vector)
	.section .data
	.long irq\number
.endm

.globl isr_vector
isr_vector:
	.set i, 0
	.rept 32
		irq_insert %i
		.set i, i+1
	.endr
	
