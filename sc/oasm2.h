#if __AS09__
# asm
	LEAU	,X
	LDX	_outbuftop,PC
	PSHS	X
	LDX	_outbufptr,PC
	BRA	OUTSTR.NEXT

CALL.FLUSHOUT
	PSHS	U,B
	STX	_outbufptr,PC
	LBSR	_flushout
	LDX	_outbufptr,PC
	LDY	_outbuftop,PC
	PULS	B,U,PC

OUTSTR.LOOP
	STB	,X+
	CMPX	,S
	BLO	OUTSTR.NEXT
	BSR	CALL.FLUSHOUT
	STY	,S
OUTSTR.NEXT
	LDB	,U+
	BNE	OUTSTR.LOOP
	STX	_outbufptr,PC
	LEAS	2,S
# endasm
#endif /* __AS09__ */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
	mov	dx,di
	mov	cx,si
# endif
# if !__FIRST_ARG_IN_AX__
	pop	ax
	pop	si
	dec	sp
	dec	sp
	push	ax
# else
#  if ARGREG == DREG
	xchg	si,ax
#  else
	mov	si,bx
#  endif
# endif
	mov	di,[_outbufptr]
	mov	bx,[_outbuftop]
	br	OUTSTR.NEXT

CALL.FLUSHOUT:
	push	si
# if !__CALLER_SAVES__
	push	dx
	push	cx
# endif
	push	ax
	mov	[_outbufptr],di
	call	_flushout
	mov	di,[_outbufptr]
	mov	bx,[_outbuftop]
	pop	ax
# if !__CALLER_SAVES__
	pop	cx
	pop	dx
#endif
	pop	si
	ret

OUTSTR.LOOP:
	stosb
	cmp	di,bx
	jb	OUTSTR.NEXT
	call	CALL.FLUSHOUT
OUTSTR.NEXT:
	lodsb
	test	al,al
	jne	OUTSTR.LOOP
	mov	[_outbufptr],di
# if !__CALLER_SAVES__
	mov	si,cx
	mov	di,dx
# endif
# endasm
#endif /* __AS386_16__ */

#if __AS386_32__
# asm
# if !__CALLER_SAVES__
	mov	edx,edi
	push	esi
#  define TEMPS 4
# else
#  define TEMPS 0
# endif
# if !__FIRST_ARG_IN_AX__
	mov	esi,TEMPS+_outstr.s[esp]
# else
#  if ARGREG == DREG
	xchg	esi,eax
#  else
	mov	esi,ebx
#  endif
# endif
	mov	edi,[_outbufptr]
	mov	ecx,[_outbuftop]
	br	OUTSTR.NEXT

CALL.FLUSHOUT:
	push	esi
# if !__CALLER_SAVES__
	push	edx
# endif
	push	eax
	mov	[_outbufptr],edi
	call	_flushout
	mov	edi,[_outbufptr]
	mov	ecx,[_outbuftop]
	pop	eax
# if !__CALLER_SAVES__
	pop	edx
# endif
	pop	esi
	ret

OUTSTR.LOOP:
	stosb
	cmp	edi,ecx
	jb	OUTSTR.NEXT
	call	CALL.FLUSHOUT
OUTSTR.NEXT:
	lodsb
	test	al,al
	jne	OUTSTR.LOOP
	mov	[_outbufptr],edi
# if !__CALLER_SAVES__
	pop	esi
	mov	edi,edx
# endif
# endasm
#endif /* __AS386_32__ */
