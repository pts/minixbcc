| leorl.s

	.globl	leorl
	.globl	leorul
	.text
	.even

leorl:
leorul:
if __IBITS__ = 32
error unneeded
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	xor	ax,[di]
	xor	bx,[di+2]
endif
	ret
