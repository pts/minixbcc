| lsubb.s

	.globl	lsubb
	.globl	lsubub
	.text
	.even

lsubb:
lsubub:
	sub	bx,[di+2]
	sbb	ax,[di]
	ret
