| divsi3.s

	.globl ___divsi3
	.text
	.align	4

 ___divsi3:
	push	edx
	mov	eax,[esp+4+4]
	cdq
	idiv	[esp+4+4+4]
	pop	edx
	ret

	.globl ___udivsi3
	.text
	.align	4

 ___udivsi3:
	push	edx
	mov	eax,[esp+4+4]
	sub	edx,edx
	div	[esp+4+4+4]
	pop	edx
	ret
