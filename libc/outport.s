| void outport( int port, int value );
| writes the word  value  to  the i/o port  port

	.globl	_outport
	.text
	.even
_outport:
	pop	bx
	pop	dx
	pop	ax
	sub	sp,*4
	outw
	jmp	bx
