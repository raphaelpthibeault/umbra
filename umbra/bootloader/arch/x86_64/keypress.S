	.section .realmode
	.code16

	/* PIT interrupt hook */
int_08_ticks_counter:
	.long 0	
int_08_callback:
	.long 0
	/* key press info */
key_mods:
	.long 0
key_ascii:
	.long 0
key_scan:
	.long 0

int_08_isr:
	pushf
	incl int_08_ticks_counter
	popf
	ljmp *int_08_callback

	.code32
	.globl pit_sleep_and_quit_on_keypress
	.extern getchar_internal
	/* ticks in %eax per function signature:
	 * int pit_sleep_and_quit_on_keypress(uint32_t ticks)
	 * __attribute__((regparm(1)));
	 * */
pit_sleep_and_quit_on_keypress:
	/* 
	 * Note, %eax is not saved because the expectation is that (per C calling convention) the return value will be in %eax
	 * */
	pushl %ebp
	pushl	%ecx
	pushl %ebx
	pushl %esi
	pushl %edi	
	pushl	%edx

	/* hook int 8 (PIT) */
	# save the original interrupt vector
	movl    (0x08 * 4), %edx
	movl    %edx, int_08_callback
	# instal  new ISR
	movl    $int_08_isr, (0x08 * 4)

	/* put ticks in edx */
	movl %eax, %edx

	/* zero ticks counter and keypress results */	
	movl $0, int_08_ticks_counter
	movl $0, key_mods
	movl $0, key_ascii
	movl $0, key_scan
	
	/* switch to real mode */	
	call prot_to_real
	.code16

	/* enable interrupts to receive PIT ticks */
	sti
	
	/* loop til tick or timeout */
.pit_tick_loop:
	/* check timeout */
	cmpl %edx, int_08_ticks_counter	
	je .pit_tick_loop_end	

	/* check keypress */
	movw $0x0100, %ax
	int $0x16
	/* jz (zero flag set) means no key is in the buffer */
	jz .pit_tick_loop

	/* key was pressed, get the info */
	movw $0x0000, %ax
	int $0x16
	movb %al, key_ascii
	movb %ah, key_scan

	/* check for modifier keys */
	movw $0x0200, %ax
	int $0x16
	/* test if the 3rd bit (Ctrl) is set */
	testb $0x04, %al
	jz .pit_tick_loop_end # if not Ctrl, done

	/* handle Ctrl key: modify ASCII for combos like Ctrl+C */
	movb $0x04, key_mods
	addb $0x60, key_ascii

.pit_tick_loop_end:
	cli

	call real_to_prot
	.code32
	/* restore the original PIT interrupt handler */
	movl int_08_callback, %edx
	movl %edx, (0x08 * 4)
	/* check if a key was actually pressed (scan code is not zero) */
	cmpl $0, key_scan
	je .fail
	
	/* call getchar_internal 
	 * signature: int getchar_internal(uint8_t scancode, uint8_t ascii, uint32_t shift_state)
	 * */
	pushl key_mods
	pushl key_ascii
	pushl key_scan
	call getchar_internal

	addl $12, %esp # clean up the 3 arguments from the stack

	jmp .exit

.fail:
	/* e.g. timeout */
	xorl %eax, %eax

.exit:
	/* restore registers */
	popl	%edx
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ecx
	popl	%ebp
	ret

