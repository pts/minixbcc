| leorl.s

	.globl	leorl
	.globl	leorul
	.text
	.even

leorl:
leorul:
	xor	ax,[di]
	xor	bx,[di+2]
	ret
