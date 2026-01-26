| isr.s
| isr doesn't preserve cl

	.globl isr
	.text
	.even

isr:
	mov	cl,bl
if __IBITS__ = 32
	sar	eax,cl
else  | if __IBITS__ = 32
	sar	ax,cl
endif  | else if __IBITS__ = 32
	ret
