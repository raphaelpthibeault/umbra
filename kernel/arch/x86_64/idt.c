#include <arch/x86_64/idt.h>
#include <arch/x86_64/isr.h>
#include <arch/x86_64/tss.h>
#include <arch/x86_64/memory_references.h>
#include <drivers/serial.h>

/* defined in bootstrap.S (in .bss) */
extern struct idt_gate IDT_table[];
extern struct idtr idtr64;

/* defined here */
static struct idtr idtr = {0}; // should be in .bss

static void
create_trap_gate(uint8_t vector, uintptr_t isr, uint16_t cs, uint8_t dpl)
{
	struct idt_gate *gate = &IDT_table[vector];

	gate->offset_1 = (isr & 0xFFFF);
	gate->selector = cs;
	gate->ist = 0; // ?
	gate->attributes = IDT_DESCRIPTOR_X32_TRAP | dpl | IDT_DESCRIPTOR_PRESENT; /* type = IDT_DESCRIPTOR_X32_TRAP, privilege = dpl, present = IDT_DESCRIPTOR_PRESENT */
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

	create_trap_gate(0x00, (uintptr_t)&divide_error_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x01, (uintptr_t)&debug_interrupt_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x02, (uintptr_t)&non_maskable_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x03, (uintptr_t)&break_point_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x04, (uintptr_t)&overflow_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x05, (uintptr_t)&array_bounds_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x06, (uintptr_t)&invalid_opcode_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x07, (uintptr_t)&device_not_available_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x08, (uintptr_t)&double_fault_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x09, (uintptr_t)&coproc_seg_overrun_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x0a, (uintptr_t)&invalid_tss_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x0b, (uintptr_t)&segment_not_present_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x0c, (uintptr_t)&stack_fault_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x0d, (uintptr_t)&general_protection_fault_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x0e, (uintptr_t)&page_fault_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x10, (uintptr_t)&coproc_error_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x11, (uintptr_t)&alignment_check_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x12, (uintptr_t)&machine_check_wrapper, KERNEL_CODE, 0);
	create_trap_gate(0x13, (uintptr_t)&sse_fault_wrapper, KERNEL_CODE, 0);

	/* no need to reload, changes to IDT apply immediately */
	serial_print("IDT: idt assembled\n");	
}

