| setjmp.s
| int setjmp(jmp_buf jb);
| int longjmp(jmp_buf jb, int value);

	.define	_setjmp
	.define	_longjmp

if __IBITS__ = 32
FIRSTARGINREG	=	0		| nonzero if first arg is in eax

EBP_OFF		=	0
ESP_OFF		=	4
EIP_OFF		=	8

	.text
	.align	4
_setjmp:
	pop	ecx			| eip
if	FIRSTARGINREG = 0
	mov	edx,0[esp]
endif
	mov	EBP_OFF[edx],ebp
	mov	ESP_OFF[edx],esp
	mov	EIP_OFF[edx],ecx
	sub	eax,eax			| non-jump return
	jmp	ecx

	.align	4
_longjmp:
if	FIRSTARGINREG
	mov	edx,eax			| jb
	mov	eax,4[esp]		| value
else
	mov	edx,4[esp]		| jb
	mov	eax,8[esp]		| value
endif
	cmp	eax,#1			| fixup 0 value to 1 (avoid branches)
	adc	al,#0			| change eax to 1 iff it was 0
	mov	ebp,EBP_OFF[edx]
	mov	esp,ESP_OFF[edx]
	jmp	EIP_OFF[edx]
else  | if __IBITS__ = 32
BP_OFF		=	0
SP_OFF		=	2
IP_OFF		=	4

	.text
	.even
_setjmp:
	pop	cx		| ip
	pop	bx		| pointer to jmp_buf
	dec	sp
	dec	sp
	mov	[bx+BP_OFF],bp
	mov	[bx+SP_OFF],sp
	mov	[bx+IP_OFF],cx
	xor	ax,ax		| non-jump return
	jmp	cx

	.even
_longjmp:
	pop	bx		| junk ip
	pop	bx		| pointer to jmp_buf
	pop	ax		| value (sp now junk, don't fake args)
	cmp	ax,#1		| fixup 0 value to 1 (avoiding branches)
	adc	al,#0		| change ax to 1 iff ax was 0

| the  setjmp.s  in  MINIX 1.2  backtraces the frame pointers here
| this can't be done with more efficient nonstandard frames

	mov	bp,[bx+BP_OFF]
	mov	sp,[bx+SP_OFF]
	jmp	[bx+IP_OFF]
endif  | else if __IBITS__ = 32
