| landl.s

	.globl	landl
	.globl	landul
	.text
	.even

landl:
landul:
	and	ax,[di]
	and	bx,[di+2]
	ret
