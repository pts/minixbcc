#if __AS386_16__
# asm
# if !__FIRST_ARG_IN_AX__
	pop	dx
	pop	ax
	dec	sp
	dec	sp
# else
#  if ARGREG != DREG
	xchg	ax,bx
#  endif
# endif
	mov	bx,[_outbufptr]
	mov	[bx],al
	inc	bx
	mov	[_outbufptr],bx
	cmp	bx,[_outbuftop]
	jae	OUTBYTE.FLUSH
# if !__FIRST_ARG_IN_AX__
	jmp	dx
# else
	ret
# endif

OUTBYTE.FLUSH:
# if !__FIRST_ARG_IN_AX__
	push	dx
# endif
	br	_flushout
# endasm
#endif /* __AS386_16__ */

#if __AS386_32__
# asm
# if !__FIRST_ARG_IN_AX__
	mov	eax,_outbyte.c[esp]
# else
#  if ARGREG != DREG
	xchg	eax,ebx
#  endif
# endif
	mov	ecx,[_outbufptr]
	mov	[ecx],al
	inc	ecx
	mov	[_outbufptr],ecx
	cmp	ecx,[_outbuftop]
	jae	OUTBYTE.FLUSH
	ret

OUTBYTE.FLUSH:
	br	_flushout
# endasm
#endif /* __AS386_32__ */
