if __IBITS__ = 32
| isl.s
| isl, islu don't preserve cl

	.globl isl
	.globl islu
	.text
	.even

isl:
islu:
	mov	cl,bl
	shl	eax,cl
	ret
else  | if __IBITS__ = 32
| isl.s
| isl, islu don't preserve cl

	.globl isl
	.globl islu
	.text
	.even

isl:
islu:
	mov	cl,bl
	shl	ax,cl
	ret
endif  | else if __IBITS__ = 32
