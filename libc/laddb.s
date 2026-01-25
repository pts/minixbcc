| laddb.s

	.globl	laddb
	.globl	laddub
	.text
	.even

laddb:
laddub:
	add	bx,[di+2]
	adc	ax,[di]
	ret
