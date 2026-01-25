if __IBITS__ = 32
| isru.s
| isru doesn't preserve cl

	.globl isru
	.text
	.even

isru:
	mov	cl,bl
	shr	eax,cl
	ret
else  | if __IBITS__ = 32
| isru.s
| isru doesn't preserve cl

	.globl isru
	.text
	.even

isru:
	mov	cl,bl
	shr	ax,cl
	ret
endif  | else if __IBITS__ = 32
