| ldivl.s
| Divides signed 32-bit long dividend BX:AX by signed 32-bit long divisor [DI+2]:[DI], rounds towards 0, saves the quotient to BX:AX, saves the remainder to DI:CX. Ruins DX and FLAGS. Traps on division by 0 or overflow.
.globl ldivl
ldivl:
if __IBITS__ = 32
error unneeded
else
.extern __I4D
	mov dx, [di]
	mov cx, [di+2]
	xchg dx, bx
	call __I4D
	mov di, cx
	mov cx, bx
	mov bx, dx
	ret
endif

