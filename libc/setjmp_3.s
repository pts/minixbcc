| setjmp.s
| int setjmp(jmp_buf jb);
| int longjmp(jmp_buf jb, int value);

	.define	_setjmp
	.define	_longjmp

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
