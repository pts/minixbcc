| imul.s
| imul_, imul_u don't preserve edx

	.globl imul_
	.globl imul_u
	.text
	.even

imul_:
imul_u:
	imul	ebx
	ret
