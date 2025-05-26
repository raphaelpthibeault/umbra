	.text
	.code32

	.globl boot_menu
	/* loaded at 0xf000
	 **/
boot_menu:	
	popl %eax	/* clean up stack by removing return address, we will not ret */
	call .L1 /* trick to get EIP-relative address */
.L1:
	popl %eax  /* have L1 address */
	addl $(.L3 - .L1), %eax /* %eax now has address of L3 ; L3 = (L3 - L1) + L1 */
	movl %esp, (%eax) /* put stack pointer into L3 */
	
	.extern _boot_menu
	jmp _boot_menu

.L3:
	.quad 0
