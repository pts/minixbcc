| imul.s
| imul_, imul_u don't preserve edx/dx

	.globl imul_
	.globl imul_u
	.text
	.even

imul_:
imul_u:
if __IBITS__ = 32
	imul	ebx
else  | if __IBITS__ = 32
	imul	bx
endif  | else if __IBITS__ = 32
	ret
