| void oportb( int port, char value );
| writes the byte  value  to  the i/o port  port
| this would be outportb except for feeble linkers

	.globl	_oportb
	.text
	.even
_oportb:
	pop	bx
	pop	dx
	pop	ax
	sub	sp,*4
	outb
	jmp	bx
