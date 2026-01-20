|
| true0s: Minix 1.5.10 i86 program which does exit(0).
| by pts@fazekas.hu at Tue Jan 20 13:50:03 CET 2026
|

.bss
.comm __M, 24  | message _M;
.text
.globl crtso  | Program entry point.
crtso:
.globl _main  | Prevent the error: ld no start symbol
_main:
	|mov [__M+2*2], #0  | exit_code := EXIT_SUCCESS == 0. Not needed, .bss is zero-initialized.
	movb [__M+2*1], *1  | *(char*)&_M.m_type = EXIT;
	xor ax, ax  | AX := MM (== 0).
	mov bx, #__M
	|
	mov cx, #3  | BOTH == 3. sendrec(srcdest, ptr).
	int $20  | trap to the kernel; ruins AX, BX and CX, keeps DX.
	| Not reached, syscall EXIT doesn't return.
