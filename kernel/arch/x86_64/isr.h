#ifndef __X86_64_ISR_H__
#define __X86_64_ISR_H__

#include <types.h>
#include <arch/x86_64/registers.h>

#define INTERRUPT(name) \
void user_##name(void); \
void kernel_##name(struct registers *regs); \
extern void name##_wrapper(void);

#define EXCEPTION(name) \
void user_##name(struct selector_error_code error); \
void kernel_##name(struct registers *regs, struct selector_error_code error); \
extern void name##_wrapper(void);

#define PAGEFAULT(name) \
void user_##name(struct page_fault_error_code error); \
void kernel_##name(struct registers *regs, struct page_fault_error_code error); \
extern void name##_wrapper(void);


INTERRUPT(divide_error)
INTERRUPT(debug_interrupt)
INTERRUPT(non_maskable)
INTERRUPT(break_point)
INTERRUPT(overflow)
INTERRUPT(array_bounds)
INTERRUPT(invalid_opcode)
INTERRUPT(device_not_available)
EXCEPTION(double_fault)
INTERRUPT(coproc_seg_overrun)
EXCEPTION(invalid_tss)
EXCEPTION(segment_not_present)
EXCEPTION(stack_fault)
EXCEPTION(general_protection_fault)
EXCEPTION(page_fault)
INTERRUPT(coproc_error)
EXCEPTION(alignment_check)
INTERRUPT(machine_check)
INTERRUPT(sse_fault)


#undef INTERRUPT
#undef EXCEPTION
#undef PAGEFAULT

#endif // !__X86_64_ISR_H__
