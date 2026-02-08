| lmodl.s
| Divides signed 32-bit long dividend BX:AX by signed 32-bit long divisor [DI+2]:[DI], rounds towards 0, saves the quotient to DI:CX, saves the remainder to BX:AX. Ruins DX and FLAGS. Traps on division by 0 or overflow.
.globl lmodl
lmodl:
if __IBITS__ = 32
error unneeded
else
.extern __I4D
	mov dx, [di]
	mov cx, [di+2]
	xchg dx, bx
	call __I4D
	mov di, dx
	xchg cx, ax
	xchg bx, ax
	ret
endif
