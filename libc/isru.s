| isru.s
| isru doesn't preserve cl

	.globl isru
	.text
	.even

isru:
	mov	cl,bl
if __IBITS__ = 32
	shr	eax,cl
else  | if __IBITS__ = 32
	shr	ax,cl
endif  | else if __IBITS__ = 32
	ret
