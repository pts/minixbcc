| int inportb( int port );
| reads a byte from the i/o port  port  and returns it

	.globl	_inportb
	.text
	.even
_inportb:
	pop	bx
	pop	dx
	dec	sp
	dec	sp
	in
	sub	ah,ah
	jmp	bx
