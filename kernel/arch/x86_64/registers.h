#ifndef __X86_64_REGISTERS_H__
#define __X86_64_REGISTERS_H__

#include <types.h>

struct registers
{
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t rsp;
	uint64_t rbx;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rax;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
} __attribute__((packed));

struct selector_error_code
{
	uint64_t external : 1;
	uint64_t descriptor_IDT : 1;
	uint64_t descriptor_LDT : 1;
	uint64_t selector_index : 13;
	uint64_t reserved : 48;
} __attribute__((packed));

struct page_fault_error_code
{
	uint64_t present : 1;
	uint64_t write_access : 1;
	uint64_t user_mode : 1;
	uint64_t reserved_bits : 1;
	uint64_t instruction_fetch : 1;
	uint64_t reserved : 59;
} __attribute__((packed));

#endif // !__X86_64_REGISTERS_H__
