if __IBITS__ = 32
| isr.s
| isr doesn't preserve cl

	.globl isr
	.text
	.even

isr:
	mov	cl,bl
	sar	eax,cl
	ret
else  | if __IBITS__ = 32
| isr.s
| isr doesn't preserve cl

	.globl isr
	.text
	.even

isr:
	mov	cl,bl
	sar	ax,cl
	ret
endif  | else if __IBITS__ = 32
