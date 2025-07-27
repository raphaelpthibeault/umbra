#include <memory_references.h>

#define PUSHALL \
	pushq %rax; \
	pushq %rcx; \
	pushq %rdx; \
	pushq %rbx; \
	pushq %rsp; \
	addq $32, 0(%rsp); \
	pushq %rbp; \
	pushq %rsi; \
	pushq %rdi; \
	pushq %r8; \
	pushq %r9; \
	pushq %r10; \
	pushq %r11; \
	pushq %r12; \
	pushq %r13; \
	pushq %r14; \
	pushq %r15;

#define POPALL \
	popq %r15; \
	popq %r14; \
	popq %r13; \
	popq %r12; \
	popq %r11; \
	popq %r10; \
	popq %r9; \
	popq %r8; \
	popq %rdi; \
	popq %rsi; \
	popq %rbp; \
	addq $8, %rsp; \
	popq %rbx; \
	popq %rdx; \
	popq %rcx; \
	popq %rax;

/*  this gets complicated
 *  want to support both User and Kernel interrupts/exceptions
 *  what do I call it? "wrapper" that decides which one
 *  e.g. DivideErrorWrapper with DivideErrorUser and DivideErrorKernel
 * */

