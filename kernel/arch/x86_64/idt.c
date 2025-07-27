#include <arch/x86_64/idt.h>
#include <arch/x86_64/memory_references.h>
#include <drivers/serial.h>

enum gate_type
{
	INTERRUPT_GATE_TYPE = 0xF,
	TRAP_GATE_TYPE = 0xE,
};

__attribute__((section(".init.text"))) static void 
create_trap_gate(struct idt_gate *gate, uint16_t code_segment, void (*ip)(void), uint8_t dpl)
{
	uintptr_t addr = (uintptr_t)ip;

	gate->offset_3 = addr >> 32;
	gate->offset_2 = addr >> 16;
	gate->offset_1 = addr;
	gate->selector = code_segment;
	gate->type = TRAP_GATE_TYPE; 
	gate->privilege = dpl;	
	gate->present = 1;
}

__attribute__((section(".init.text"))) void 
set_idt(void)
{
	/* NOTE: idtr64 already loaded by bootstrap.S */
	struct idtr *idtr = (struct idtr *)idtr64;
	serial_print("IDT:\tidtr->limit: 0x%x\n", idtr->limit);	
	serial_print("\tidtr->base: 0x%x\n", idtr->base);

	/* 4096 bytes = 256 entries * 16 bytes / entry */
	struct idt_gate *idt = (struct idt_gate *)IDT_table;

	//create_trap_gate(&idt[0x00], KERNEL_CODE, &DivideErrorWrapper);
	// with the wrapper pushing dpl to stack? because the wrapper is supposed to decide privilege (kernel / user), but in this case I know it's kernel so what am I doing?

	serial_print("IDT: set_idt\n");	
}

