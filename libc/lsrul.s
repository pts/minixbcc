| lsrul.s
| lsrul doesn't preserve cx

	.globl	lsrul
	.text
	.even

lsrul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	mov	cx,di
	jcxz	LSRU_EXIT
	cmp	cx,*32
	jae	LSRU_ZERO
LSRU_LOOP:
	shr	bx,*1
	rcr	ax,*1
	loop	LSRU_LOOP
LSRU_EXIT:
	ret

	.even

LSRU_ZERO:
	xor	ax,ax
	mov	bx,ax
endif
	ret
