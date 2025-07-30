#include <arch/x86_64/isr.h>
#include <arch/x86_64/registers.h>
#include <types.h>
#include <drivers/serial.h>

/* USER LEVEL TRAP HANDLERS */

void 
user_divide_error(void)
{
	
}

void 
user_debug_interrupt(void)
{
	
}

void
user_non_maskable(void)
{
	
}

void 
user_break_point(void)
{
	
}

void
user_overflow(void)
{
	
}

void
user_array_bounds(void)
{
	
}

void 
user_invalid_opcode(void)
{
	
}

void
user_device_not_available(void)
{
	
}

void
user_double_fault(struct selector_error_code error)
{
	
}

void
user_coproc_seg_overrun(void)
{
	
}

void
user_invalid_tss(struct selector_error_code error)
{
	(void)error;	
}

void
user_segment_not_present(struct selector_error_code error)
{
	(void)error;	
}

void
user_stack_fault(struct selector_error_code error)
{
	(void)error;	
}

void
user_general_protection_fault(struct selector_error_code error)
{
	(void)error;
}

void
user_page_fault(struct selector_error_code error)
{
	(void)error;
}

void
user_coproc_error(void)
{
	
}

void
user_alignment_check(struct selector_error_code error)
{
	(void)error;
}

void
user_machine_check(void)
{
	
}

void 
user_sse_fault(void)
{
	
}

/* KERNEL LEVEL TRAP HANDLERS */

void
kernel_divide_error(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_divide_error occurred\n");
}

void
kernel_debug_interrupt(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_debug_interrupt occurred\n");
}

void
kernel_non_maskable(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_non_maskable occurred\n");
}

void
kernel_break_point(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_break_point occurred\n");
}

void
kernel_overflow(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_overflow occurred\n");
}

void
kernel_array_bounds(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_array_bounds occurred\n");
}

void
kernel_invalid_opcode(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_invalid_opcode occurred\n");
}

void
kernel_device_not_available(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_device_not_available occurred\n");
}

void
kernel_double_fault(struct registers *regs, struct selector_error_code error)
{
	(void)regs;
	(void)error;
	serial_print("[ISR] kernel_double_fault occurred\n");
}

void
kernel_coproc_seg_overrun(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_coproc_seg_overrun occurred\n");
}

void
kernel_invalid_tss(struct registers *regs, struct selector_error_code error)
{
	(void)regs;
	(void)error;
	serial_print("[ISR] kernel_invalid_tss occurred\n");
}

void
kernel_segment_not_present(struct registers *regs, struct selector_error_code error)
{
	(void)regs;
	(void)error;
	serial_print("[ISR] kernel_segment_not_present occurred\n");
}

void
kernel_stack_fault(struct registers *regs, struct selector_error_code error)
{
	(void)regs;
	(void)error;
	serial_print("[ISR] kernel_stack_fault occurred\n");
}

void
kernel_general_protection_fault(struct registers *regs, struct selector_error_code error)
{
	(void)regs;
	(void)error;
	serial_print("[ISR] kernel_general_protection_fault occurred\n");
}

void
kernel_page_fault(struct registers *regs, struct selector_error_code error)
{
	(void)regs;
	(void)error;
	serial_print("[ISR] kernel_page_fault occurred\n");
}

void
kernel_coproc_error(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_coproc_error occurred\n");
}

void
kernel_alignment_check(struct registers *regs, struct selector_error_code error)
{
	(void)regs;
	(void)error;
	serial_print("[ISR] kernel_alignment_check occurred\n");
}

void
kernel_machine_check(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_machine_check occurred\n");
}

void
kernel_sse_fault(struct registers *regs)
{
	(void)regs;
	serial_print("[ISR] kernel_sse_fault occurred\n");
}

