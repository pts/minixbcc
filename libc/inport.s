| int inport( int port );
| reads a word from the i/o port  port  and returns it

	.globl	_inport
	.text
	.even
_inport:
	pop	bx
	pop	dx
	dec	sp
	dec	sp
	inw
	jmp	bx
