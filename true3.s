|
| true0s: Minix 1.5.10 i386 program which does exit(0).
| by pts@fazekas.hu at Tue Jan 20 13:50:03 CET 2026
|

.bss
.comm __M, 36  | message _M;
.text
.globl crtso  | Program entry point.
crtso:
.globl _main  | Prevent the error: ld no start symbol
_main:
	|mov [__M+4*2], #0  | exit_code := EXIT_SUCCESS == 0. Not needed, .bss is zero-initialized.
	movb [__M+4*1], *1  | *(char*)&_M.m_type = EXIT;
	xor eax, eax  | EAX := MM (== 0).
	mov ebx, #__M
	xor ecx, ecx
	mov cl, *3  | BOTH == 3. sendrec(srcdest, ptr).
	int $21  | trap to the kernel; ruins EAX, EBX and ECX, keeps EDX.
	| Not reached, syscall EXIT doesn't return.
