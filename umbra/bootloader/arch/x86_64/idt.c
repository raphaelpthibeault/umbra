#include "idt.h"
#include <types.h>
#include <lib/misc.h>
#include "asm.h"

#define IDT_ENTRIES_SIZE 32

static struct idt_gate idt[IDT_ENTRIES_SIZE];

// set in real mode space recall prot cs=0x08, ds=0x10; real cs=0x18, ds=0x20
struct idtr idtr = {
	IDT_ENTRIES_SIZE * sizeof(struct idt_gate) - 1, //sizeof(idt) - 1, ?
	(uintptr_t)idt, // base
};


static void 
set_idt_gate(uint8_t vec, void *isr, uint8_t type)
{
	uintptr_t isr_addr = (uintptr_t)isr;
	idt[vec].offset_lo = (uint16_t)isr_addr;
	idt[vec].cs_selector = PSEUDO_REAL_CSEG;
	idt[vec].unused = 0;
	idt[vec].attributes = type;
	idt[vec].offset_hi = (uint16_t)(isr_addr >> 16);
}

extern void *isr_vector[];

void
set_idt(void) 
{
	/* load bootloader idt instead of the stub protidt in core.S */
	for (uint8_t v = 0; v < IDT_ENTRIES_SIZE; ++v) {
		set_idt_gate(v, isr_vector[v], 0x8e);
	}
	
	__asm__ volatile ("lidt %0" :: "m"(idtr) : "memory");
}

