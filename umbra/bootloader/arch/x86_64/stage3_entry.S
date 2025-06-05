	.text
	.code32
	.globl stage3_start
	/* loaded at 0xf000
	 **/
stage3_start: 
	popl %eax	/* clean up stack by removing return address, we will not ret */
	call .L1 /* trick to get EIP-relative address */
.L1:
	popl %eax  /* have L1 address */
	addl $(.L3 - .L1), %eax /* %eax now has address of L3 ; L3 = (L3 - L1) + L1 */
	movl %esp, (%eax) /* put stack pointer into L3 */

	.extern _stage3_start
	jmp _stage3_start

.L3:
	.quad 0
