#ifndef __IDT_H__
#define __IDT_H__

#include <types.h>

struct idtr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct idt_gate {
	uint16_t offset_lo;
	uint16_t cs_selector;
	uint8_t unused;
	uint8_t attributes;
	uint16_t offset_hi;
} __attribute__((packed));

extern struct idtr idtr;

void set_idt(void);

#endif // !__IDT_H__
