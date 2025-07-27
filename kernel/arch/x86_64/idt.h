#ifndef __x86_64_IDT_H__
#define __x86_64_IDT_H__

#include <types.h>

/* NOTE: idtr64 in bootstrap.S (i.e. already in .bss) ! */
extern char IDT_table[];
extern char idtr64[];

struct idtr
{
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

struct idt_gate
{
	uint16_t offset_1;		/* offset bits 0..15 */
	uint16_t selector;		/* code segment selector in gdt or ldt */
	uint8_t ist;					/* bits 0..2 holds Interrupt Stack Table offset, rest of bits zero (reserved) */
	uint8_t type:4;				/* gate type */
	uint8_t code_or_data:1; /* */
	uint8_t privilege:2;	/* defines the CPU Privilege Levels which are allowed to access this interrupt via the INT instruction */
	uint8_t present:1;		/* */
	uint16_t offset_2;		/* offset bits 16..31 */
	/* 64-bit */
	uint32_t offset_3;		/* offset bits 32..63 */
	uint32_t zero;				/* reserved */
} __attribute__((packed));

void set_idt(void);

#endif // !__x86_64_IDT_H__
