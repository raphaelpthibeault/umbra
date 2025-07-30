#ifndef __x86_64_IDT_H__
#define __x86_64_IDT_H__

#include <types.h>

#define IDT_NUM_DESCRIPTORS						256
#define IDT_NUM_CPU_EXCEPTIONS				32

#define IDT_DESCRIPTOR_X16_INTERRUPT	0x06
#define IDT_DESCRIPTOR_X16_TRAP				0x07
#define IDT_DESCRIPTOR_X32_TASK				0x05
#define IDT_DESCRIPTOR_X32_INTERRUPT  0x0E
#define IDT_DESCRIPTOR_X32_TRAP				0x0F
#define IDT_DESCRIPTOR_RING1					0x40
#define IDT_DESCRIPTOR_RING2					0x20
#define IDT_DESCRIPTOR_RING3					0x60
#define IDT_DESCRIPTOR_PRESENT				0x80

#define IDT_DESCRIPTOR_EXCEPTION			(IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)
#define IDT_DESCRIPTOR_EXTERNAL				(IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)
#define IDT_DESCRIPTOR_CALL						(IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT | IDT_DESCRIPTOR_RING3)

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
	uint8_t attributes;		/* type (4), code_or_data (1), privilege (2), present (1) */
	uint16_t offset_2;		/* offset bits 16..31 */
	/* 64-bit */
	uint32_t offset_3;		/* offset bits 32..63 */
	uint32_t zero;				/* reserved */
} __attribute__((packed));

void idt_assemble(void);

#endif // !__x86_64_IDT_H__
