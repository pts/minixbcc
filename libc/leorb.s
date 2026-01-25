| leorb.s

	.globl	leorb
	.globl	leorub
	.text
	.even

leorb:
leorub:
	xor	ax,[di]
	xor	bx,[di+2]
	ret
