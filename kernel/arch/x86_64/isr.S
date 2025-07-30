	#include <memory_references.h>

.altmacro

.macro PUSHALL
	pushq %rax
	pushq %rcx
	pushq %rdx
	pushq %rbx
	pushq %rsp
	addq $32, 0(%rsp)
	pushq %rbp
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

.macro POPALL
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
	popq %rbp
	addq $8, %rsp
	popq %rbx
	popq %rdx
	popq %rcx
	popq %rax
.endm

.macro INTERRUPT name
	.globl \name\()_wrapper
	.type \name\()_wrapper, @function
	.align 16
	.extern user_\name
	.extern kernel_\name
\name\()_wrapper:
	cli
	testq $3, 8(%rsp) # test if interrupt came from user mode (CPL = 3)
	jz 1f # if not user, jump to kernel branch
	# USER BRANCH
	swapgs
	movq %rax, %gs:8
	movq %rbx, %gs:16
	movq %rcx, %gs:24
	movq %rdx, %gs:32
	movq %rsi, %gs:40
	movq %rdi, %gs:48
	movq %rbp, %gs:56
	movq %r8, %gs:72
	movq %r9, %gs:80
	movq %r10, %gs:88
	movq %r11, %gs:96
	movq %r12, %gs:104
	movq %r13, %gs:112
	movq %r14, %gs:120
	movq %r15, %gs:128
	movq 24(%rsp), %rax
	movq %rax, %gs:64
	movq 16(%rsp), %rax
	movq %rax, %gs:136
	movq 0(%rsp), %rax
	movq %rax, %gs:0
	swapgs
	addq $40, %rsp
	movabsq $(user_\name), %rax
	call *%rax
	jmp return_to_user
1: # KERNEL BRANCH
	PUSHALL
	movq %rsp, %rdi
	movabsq $(kernel_\name), %rax
	call *%rax
	POPALL
	hlt 
.size \name\()_wrapper, . - \name\()_wrapper
.endm

.macro EXCEPTION name
	.globl \name\()_wrapper
	.type \name\()_wrapper, @function
	.align 16
	.extern user_\name
	.extern kernel_\name
\name\()_wrapper:
	cli
	testq $3, 16(%rsp)
	jz 1f
	# USER BRANCH
	swapgs
	movq %rax, %gs:8
	movq %rbx, %gs:16
	movq %rcx, %gs:24
	movq %rdx, %gs:32
	movq %rsi, %gs:40
	movq %rdi, %gs:48
	movq %rbp, %gs:56
	movq %r8, %gs:72
	movq %r9, %gs:80
	movq %r10, %gs:88
	movq %r11, %gs:96
	movq %r12, %gs:104
	movq %r13, %gs:112
	movq %r14, %gs:120
	movq %r15, %gs:128
	movq 32(%rsp), %rax
	movq %rax, %gs:64
	movq 24(%rsp), %rax
	movq %rax, %gs:136
	movq 8(%rsp), %rax
	movq %rax, %gs:0
	swapgs
	popq %rdi
	addq $40, %rsp
	movabsq $(user_\name), %rax
	call *%rax
	jmp return_to_user
1: # KERNEL BRANCH
	xchgq %rax, 0(%rsp)
	pushq %rcx
	pushq %rdx
	pushq %rbx
	pushq %rsp
	addq $32, 0(%rsp)
	pushq %rbp
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
	movq %rsp, %rdi
	movq %rax, %rsi
	movabsq $(kernel_\name), %rax
	call *%rax
	POPALL
	hlt
.size \name\()_wrapper, . - \name\()_wrapper
.endm


	.section .text

.type return_to_user, @function
.align 16
return_to_user:
	# TODO
	jmp return_to_user
.size return_to_user, . - return_to_user

INTERRUPT divide_error
INTERRUPT debug_interrupt
INTERRUPT non_maskable
INTERRUPT break_point
INTERRUPT overflow
INTERRUPT array_bounds
INTERRUPT invalid_opcode
INTERRUPT device_not_available
EXCEPTION double_fault
INTERRUPT coproc_seg_overrun
EXCEPTION invalid_tss
EXCEPTION segment_not_present
EXCEPTION stack_fault
EXCEPTION general_protection_fault
EXCEPTION page_fault
INTERRUPT coproc_error
EXCEPTION alignment_check
INTERRUPT machine_check
INTERRUPT sse_fault

