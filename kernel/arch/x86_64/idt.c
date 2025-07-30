#include <arch/x86_64/idt.h>
#include <arch/x86_64/tss.h>
#include <arch/x86_64/memory_references.h>
#include <drivers/serial.h>

/* defined in bootstrap.S (in .bss) */
extern struct idt_gate IDT_table[];
extern struct idtr idtr64;

/* defined in isr.S */
extern uint64_t isr_stub_table[]; // or extern void *isr_stub_table[];

/* defined here */
static bool gate_set[IDT_NUM_DESCRIPTORS];
static struct idtr idtr = {0}; // should be in .bss

uint64_t __routine_handlers[IDT_NUM_DESCRIPTORS];

static void 
set_gate(uint8_t vector, uintptr_t isr, uint8_t flags, uint8_t ist)
{
	struct idt_gate *gate = &IDT_table[vector];

	gate->offset_1 = (isr & 0xFFFF);
	gate->selector = KERNEL_CODE;
	gate->ist = ist;
	gate->attributes = flags;
	gate->offset_2 = (isr >> 16) & 0xFFFF;
	gate->offset_3 = (isr >> 32) & 0xFFFFFFFF;
	gate->zero = 0;	
}

void 
idt_assemble(void)
{
	/* NOTE: idtr64 already loaded by bootstrap.S, idtr is just a copy 
	 * to avoid going to section .init.text from section .text 
	 * IDT_table is in .bss so don't bother copying */

	idtr.limit = idtr64.limit;
	idtr.base = idtr64.base;

	for (uint8_t vector = 0; vector < IDT_NUM_CPU_EXCEPTIONS; ++vector)
	{
		//set_gate(vector, isr_stub_table[vector], IDT_DESCRIPTOR_EXCEPTION, TSS_IST_EXCEPTION);
		set_gate(vector, isr_stub_table[vector], IDT_DESCRIPTOR_EXCEPTION, 0);
		gate_set[vector] = true;
	}

	/* no need to reload, changes to IDT apply immediately */
	serial_print("IDT: idt assembled\n");	
}

