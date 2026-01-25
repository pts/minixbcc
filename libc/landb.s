| landb.s

	.globl	landb
	.globl	landub
	.text
	.even

landb:
landub:
	and	ax,[di]
	and	bx,[di+2]
	ret
