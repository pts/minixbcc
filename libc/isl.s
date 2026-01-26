| isl.s
| isl, islu don't preserve cl

	.globl isl
	.globl islu
	.text
	.even

isl:
islu:
	mov	cl,bl
if __IBITS__ = 32
	shl	eax,cl
else  | if __IBITS__ = 32
	shl	ax,cl
endif  | else if __IBITS__ = 32
	ret
