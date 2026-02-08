| ldivmod.s - 32 over 32 to 32 bit division and remainder for 8086
|
| The original ldivmod and ludivmod implementations by Bruce Evans round
| down, this implementation rounds towards 0. Rounding towards 0 and trapping
| on division by 0 or overflow are consistent with C99 (C89 allows either) and
| the x86 idiv instruction used in idiv.s and
| imod.s for 16-bit integers.
|

if __IBITS__ = 32
error unneeded
else

| Divides signed 32-bit long dividend DX:AX by signed 32-bit long divisor CX:BX, rounds towards 0, saves the quotient to DX:AX, saves the remainder to CX:BX. Ruins FLAGS. Traps on division by 0 or overflow.
| Based on OpenWatcom v2 libc.
.globl __I4D
__I4D:
	test dx, dx
	js i4ddividendneg
	test cx, cx
	jns __U4D
	neg cx
	neg bx
	sbb cx, *0
	call __U4D
	j i4dqneg
i4ddividendneg:
	neg dx
	neg ax
	sbb dx, *0
	test cx, cx
	jns i4drneg
	neg cx
	neg bx
	sbb cx, *0
	call __U4D
	neg cx
	neg bx
	sbb cx, *0
	ret
i4drneg:
	call __U4D
	neg cx
	neg bx
	sbb cx, *0
i4dqneg:
	neg dx
	neg ax
	sbb dx, *0
	ret

| Divides unsigned 32-bit long dividend DX:AX by unsigned 32-bit long divisor CX:BX, rounds towards 0 (same as rounding down for unsigned) saves the quotient to DX:AX, saves the remainder to CX:BX. Ruins FLAGS. Traps on division by 0 or overflow.
| Based on OpenWatcom v2 libc.
.globl __U4D
__U4D:
	test cx, cx
	jnz u4dlargedivisor  | Jumps iff the divisor doesn't fit to a 16-bit word.
	dec bx
	je u4dretsmall  | If dividing by 1, it's a no-op, keep the quotient in DX:AX, and keep the remainder 0 in CX:BX.
	inc bx
	cmp bx, dx
	ja u4dlow  | Jumps iff the quotient fits to a 16-bit word, i.e. if the divisor BX is larger than high word of the dividend (DX). If it jumps, it stores the high word of the quotient in CX, which is 0.
	mov cx, ax
	mov ax, dx
	xor dx, dx
	div bx  | This traps on division by zero.
	xchg ax, cx
u4dlow:
	div bx
	mov bx, dx  | BX := low word of the remainder.
	mov dx, cx  | DX := high word of the quotient.
	xor cx, cx  | DX := high word of the remainder. It's 0, because the remainder is smaller than the divisor, and the divisor fits to a 16-bit word, so the remainder also does.
u4dretsmall:
	ret
u4dlargedivisor:
	cmp cx, dx
	jb u4dqnonzero  | Jumps iff the high word of the dividend is larger than the high word of the divisor.
	jne u4dqzero
	cmp bx, ax
	ja u4dqzero  | Jumps iff the divisor is larger than the dividend.
u4dqone:
	sub ax, bx  | AX := low word of the remainder.
	xchg bx, ax  | BX := AX| AX := junk.
	xor cx, cx  | Set high word of the remainder to 0.
	mov ax, *1
	cwd  | Set the quotient (DX:AX) to 1.
	ret
u4dqzero:
	xor cx, cx
	xor bx, bx  | CX:BX := 0.
	xchg ax, bx
	xchg dx, cx  | Set the remainder (CX:BX) to the dividend, and the quotient (DX:AX) to 0.
	ret
u4dqnonzero:
	push bp
	push si
	xor si, si
	mov bp, si
loc8:
	add bx, bx
	adc cx, cx
	jb loc11
	inc bp
	cmp cx, dx
	jb loc8
	ja loc9
	cmp bx, ax
	jbe loc8
loc9:
	clc
loc10:
	adc si, si
	dec bp
	js loc14
loc11:
	rcr cx, *1
	rcr bx, *1
	sub ax, bx
	sbb dx, cx
	cmc
	jb loc10
loc12:
	add si, si
	dec bp
	js loc13
	shr cx, *1
	rcr bx, *1
	add ax, bx
	adc dx, cx
	jae loc12
	j loc10
loc13:
	add ax, bx
	adc dx, cx
loc14:
	mov bx, ax
	mov cx, dx
	mov ax, si
	xor dx, dx
	pop si
	pop bp
	ret

endif  | else if __IBITS__ = 32
