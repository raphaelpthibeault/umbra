#include <types.h>
#include <multiboot2.h>
#include <drivers/serial.h>
#include <arch/x86_64/idt.h>

int 
kmain(uint32_t magic, uint32_t mbi_phys)
{
	serial_print("\n\n---------- UMBRA KERNEL ----------\n");
	//serial_print("Multiboot magic: 0x%x\n", magic);
	//serial_print("Multiboot info location (physical): 0x%x\n", mbi_phys);

	if (magic != (uint32_t)0x36d76289)
	{
		serial_print("[PANIC] invalid magic!\n");
		while (1);
	}

	// kernel can't access physical, must map first!
	// can't access mbi so also can't access prepared fb, so we're black screen for now
	//struct multiboot2_start_tag *mbi_start = (struct multiboot2_start_tag *)((uintptr_t)mbi_phys);

	/* TODO: get CPU info ? */
	
	/* 
	 * general TODO;
	 * [x] IDT
	 * [ ] GDT ? do I need to do more than the bootstrap?
	 * [ ] map memory so can access mbi_phys
	 * */

	idt_assemble();

	struct multiboot2_start_tag *mbi_start = (struct multiboot2_start_tag *)((uintptr_t)mbi_phys);
	/* causes page fault */
	int foo = mbi_start->size;
	int bar = foo + foo;
	serial_print("mbi size: 0x%x\n", foo);

	while (1);
	
	/* yield to common_main()	(common for all architectures) ; some kernel/kernel.c or kernel/main.c ? */
	// return common_main();
	return 0;
}
