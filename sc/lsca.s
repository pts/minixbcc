|
| lcsa.c: libc good enough for linking sc compiled with the Minix 1.5.10 i86 ACK 3.1 C compiler, asld syntax
| by pts@fazekas.hu at Fri Jan 16 00:49:30 CET 2026
|

.globl crtso
.globl begtext, begdata, begbss
.extern endbss
.text
begtext:
.data
begdata:
.zerow 2  | food for null pointer bugs
.extern _end
.globl _brksize
_brksize: .word endbss
.bss
begbss:
__M: .zerow 12  | message _M;
.text

| See <minix/com.h> for C definitions
|SEND = 1
|RECEIVE = 2
BOTH = 3
SYSVEC = 32  | int 20h | int $20

| --- Startup and syscalls.

| This is the C run-time start-off routine.  It's job is to take the
| arguments as put on the stack by EXEC, and to parse them and set them up the
| way _main expects them.
.globl crtso
.extern _main
crtso:
	cld
	|sub bp, bp  | clear for backtrace of core files
	pop cx  | CX := argc.
	mov bx, sp  | BX := argv.
	|push ax  | push environ
	push bx  | push argv
	push cx  | push argc
	call _main
	|add sp, *6  | Not needed, we are exiting soon anyway.
	push ax  | push exit status
	push ax  | Fake return address for _exit.
	| Fall through to _exit.

| void exit(int exit_code)
.globl _exit
_exit:
| PUBLIC void exit(exit_code)
| int exit_code;
| {
|   *(char*)&_M.m_type = EXIT;
|   _M.m1_i1 = exit_code;
|   callx();
| }
	pop ax  | Return address. Won't be used.
	pop __M+4  | exit_code.
	movb __M+2, *1  | *(char*)&_M.m_type = EXIT;
	| Fall through to _callx.

| Send a message and get the response.  The '_M.m_type' field of the
| reply contains a value (>=0) or an error code (<0).
.globl _callx
_callx:
| PRIVATE int callx()
| {
|   int k;
| #ifdef DEBUG_MALLOC  /* Always false. */
|   k = _M.m_type;  /* syscall number. */
|   k = (k >= READ && k <= CREAT) || k == IOCTL;  /* MM (== 0) or FS (== 1). */
| #else
|   k = (_M.m_type & 17) != 1;  /* _M.m_type is syscall number. */  /* MM (== 0) or FS (== 1). */  /* This works for EXIT (MM), READ, WRITE, OPEN, CLOSE, CREAT, BRK (MM) and IOCTL. */
| #endif
|   k = sendrec(k, &_M);
|   if (k != 0) return(k);  /* send itself failed */
|   if (_M.m_type < 0) {
| #ifdef ERRNO  /* Always false. */
|     errno = -_M.m_type;
| #endif
|     return(-1);
|   }
|   return(_M.m_type);
| }
.data
.text
	mov ax, __M+2  | syscall number.
	andb al, *15
	dec ax
	jz callxmmfs  | Keep AX == MM (== 0).
	movb al, *1  | AX := FS (== 1).
callxmmfs:
	| Now AX is either MM (== 0) or FS (== 1), depending on the syscall number.
	mov bx, #__M
	mov cx, #BOTH  | sendrec(srcdest, ptr)
	int SYSVEC  | trap to the kernel; ruins AX, BX and CX, keeps DX.
	test ax, ax
	jnz callxret  | sendrec(...) itself has failed.
	or ax, __M+2  | Syscall result or -errno.
	mov __M+3, *0  | Set high byte of next syscall to 0.
	jns callxret
	| Here, if ERRNO is defined, we should set:
	|neg ax
	|mov _errno, ax
	mov ax, #-1  | Return value to indicate syscall error.
callxret:
	ret

| int read(int fd, char *buffer, unsigned nbytes);
.globl _read
_read:
	movb __M+2, *3  | *(char*)&_M.m_type = READ;
readwrite:
	mov bx, sp
	mov ax, 2(bx)  | Argument fd.
	mov __M+4, ax  | _M.m1_i1.
	mov ax, 4(bx)  | Argument buffer.
	mov __M+10, ax  | _M.m1_p1.
	mov ax, 6(bx)  | Argument nbytes.
	mov __M+6, ax  | _M.m1_i2.
	j _callx

| int write(int fd, const char *buffer, unsigned nbytes);
.globl _write
_write:
	movb __M+2, *4  | *(char*)&_M.m_type = WRITE;
	j readwrite

| int close(int fd);
.globl _close
_close:
	movb __M+2, *6  | *(char*)&_M.m_type = CLOSE;
	| Argument fd will be copied to _M.m1_i1.
callxarg1:
	mov bx, sp
	mov ax, 2(bx)  | Argument 1.
	mov __M+4, ax  | _M.m1_i1.
j_callx:
	j _callx

| int open00(const char *name);
.globl _open00
_open00:
	movb __M+2, *5  | *(char*)&_M.m_type = OPEN;
	xor ax, ax  | AX (flags) := 0. Will be saved to _M.m3_i2.
	| _M.m3_i2 := flags.  | Fall through to callm3ax.

callm3ax:
	mov __M+6, ax  | _M.m3_i2 = mode;
	| Fall through to callm3.

| int callm3(const char *name);
|
| This form of system call is used for those calls that contain at most
| one integer parameter along with a string.  If the string fits in the
| message, it is copied there.  If not, a pointer to it is passed.
callm3:
| PUBLIC int callm3(name) _CONST char *name; {
|   register unsigned k;
|   register char *rp;
|   k = strlen(name) + 1;
|   _M.m3_i1 = k;
|   _M.m3_p1 = (char *) name;
|   rp = &_M.m3_ca1[0];
|   if (k <= M3_STRING) {  /* 14. */
|     while (k--) { *rp++ = *name++; }
|   }
|   return callx();
| }
	push si  | Save.
	mov si, sp
	mov si, 4(si)  | Argument name.
	mov __M+8, si  | _M.m3_p1 = (char *) name;
	push si  | Argument name.
	call _strlen
	pop cx  | Clean up argument of _strlean above.
	inc ax  | k := strlen(name) + 1.
	mov __M+4, ax  | _M.m3_i1 = k;
	cmp ax, *14  | if (k <= M3_STRING)
	ja callm3skip
	xchg cx, ax  | CX := AX (k); AX := junk.
	xchg di, ax  | Save DI to AX.
	mov di, #__M+10  | rp = &_M.m3_ca1[0];
	rep
	movb
	xchg di, ax  | Restore DI from AX. AX := junk.
callm3skip:
	pop si  | Restore.
	j j_callx

| int creat(const char *name, mode_t mode);
.globl _creat
_creat:
	movb __M+2, *8  | *(char*)&_M.m_type = CREAT;
callm3arg2:
	mov bx, sp
	mov ax, 4(bx)  | Argument mode.
	j callm3ax  | _M.m3_i2 = mode.

| int isatty(int fd);
| int isatty(int fd);
.globl _isatty
_isatty:
| int isatty(fd) int fd; {  /* Minix 1.5--1.7.2. */
|   _M.TTY_REQUEST = 0x7408;  /* TIOCGETP == 0x7408 on Minix 1.5.10. */  /* #define TTY_REQUEST m2_i3 */
|   _M.TTY_LINE = fd;  /* #define TTY_LINE m2_i1 */
|   return(callx(FS, IOCTL) >= 0);  /* FS == 1; IOCTL == 54. */
| }
| int isatty(fd) int fd; {  /* Minix 1.7.4--2.0.4--3.2.0, merged isatty(...), tcgetattr(...) and ioctl(...) */
|  struct termios dummy;  /* sizeof(struct termios) == 36 == 0x24 on i386, == 32 == 0x20 on i86. */
|  m.TTY_REQUEST = (unsigned) (0x80245408L & ~(unsigned) 0);  /* TCGETS == (int) 0x80245408L on Minix 2.0.4 */  /* #define TTY_REQUEST COUNT */  /* #define COUNT m2_i3 */
|  m.TTY_LINE = fd;  /* #define TTY_LINE DEVICE */  /* #define DEVICE m2_i1 */
|  m.ADDRESS = (char *) &dummy;  /* #define ADDRESS m2_p1 */ 
|  return((callx(FS, IOCTL) >= 0);  /* FS == 1; IOCTL == 54. */  /* Actually, Minix does (...) == 0. */
| }
| int isatty(fd) int fd; {  /* Our implementation below, compatible with Minix 1.5--2.0.4--3.2.0. */
|   char dummy[sizeof(int) == 2 ? 32 : 36];  /* struct termios dummy; */  /* For compatibility with Minix 1.7.4--2.0.4--3.2.0. */
|   _M.TTY_REQUEST = 0x7408;  /* TIOCGETP; */
|   _M.TTY_LINE = fd;
|   if (callx(FS, IOCTL) >= 0) goto found_tty;  /* Minix 1.5--1.7.2. */
|   _M.TTY_REQUEST = (unsigned) (0x80245408L & ~(unsigned) 0);  /* TCGETS. */
|   _M.TTY_LINE = fd;
|   _M.ADDRESS = dummy;
|   if (callx(FS, IOCTL) < 0) return 0;  /* Minix 1.7.4--2.0.4--3.2.0. */
|  found_tty:
|   return(1);
| }
	| First try: Minix 1.5--1.7.2.
	movb __M+2, *54  | *(char*)&_M.m_type = IOCTL;
	mov __M+8, #29704  | #$7408  | _M.TTY_REQUEST = Minix_1_5_TIOCGETP;
	mov bx, sp
	mov ax, 2(bx)  | Argument fd.
	mov __M+4, ax  | _M.TTY_LINE = fd;
	call _callx  | if (callx() >= 0) goto isattydone;
	test ax, ax
	jns isattydone  | Jump iff found a TTY.
	| Not found a TTY for the first try. Second try: Minix 1.7.4--2.0.4--3.2.0.
	mov __M+8, #21512  | #$5408  | _M.TTY_REQUEST = Minix_1_7_2_TIOCGETP;
	mov bx, sp
	mov ax, 2(bx)  | Argument fd.
	mov __M+4, ax  | _M.TTY_LINE = fd;
	sub sp, *32  | struct termios &dummy;
	mov __M+18, sp  | _M.ADDRESS = &dummy.  | m2_p1.
	call _callx  | if (callx() >= 0) goto isattydone;
	add sp, *32  | Pop the dummy.
isattydone:  | return callx() >= 0;
	| This would be 1 byte longer.
	|test ax, ax
	|mov ax, #1
	|jns isattyret
	|dec ax  | AX := 0.
	rol ax, *1
	not ax
	and ax, *1
|isattyret:
	ret

| char *brk(char *addr);
.globl _brk
_brk:
| PUBLIC char *brk(addr) char *addr; {
|   *(char*)&_M.m_type = BRK;
|   _M.m1_p1 = addr;
|   if (callx() == 0) {
|     brksize = _M.m2_p1;
|     return((char*) 0);
|   } else {
|     return((char *) -1);
|   }
| }
	movb __M+2, *17  | *(char*)&_M.m_type = BRK;
	mov bx, sp
	mov bx, 2(bx)  | Argument addr.
	mov __M+10, bx
	call _callx
	test ax, ax
	jnz brkerror
	mov bx, __M+18  | _M.m2_p1.
	mov _brksize, bx  | brksize = _M.m2_p1;
	j brkret
brkerror:
	mov ax, #-1  | return((char *) -1);
brkret:
	ret

| --- C compiler function helpers.

.globl .dsret
.dsret:
	pop di
.globl .sret
.sret:
	pop si
.globl .cret
.cret:
	mov sp, bp
	pop bp
	ret

| --- C compiler case etc. helpers.

ECASE = 20

|.globl .fat
|.fat:
|.extern .trp
|.extern .stop
|	call .trp
|	call .stop  | Same as _exit.

.globl .loi
.loi:
	pop ax
	mov dx, si
	mov si, bx
	mov bx, ax
	mov ax, cx
	sar cx, #1
	jnb 1f
	xorb ah, ah
	lodb
	mov si, dx
	push ax
	jmp (bx)
1:
	sub sp, ax
	mov ax, di
	mov di, sp
	rep
	mov
	mov si, dx
	mov di, ax
	jmp (bx)

.globl .sti
.sti:
	mov dx, di
	pop ax
	mov di, bx
	mov bx, ax
	sar cx, #1
	jnb 1f
	pop ax
	stob
	mov di, dx
	jmp (bx)
1:
	mov ax, si
	mov si, sp
	rep
	mov
	mov sp, si
	mov di, dx
	mov si, ax
	jmp (bx)

.globl .csa2
.csa2:
	mov dx, (bx)
	sub ax, 2(bx)
	cmp ax, 4(bx)
	ja 1f
	sal ax, #1
	add bx, ax
	mov bx, 6(bx)
	test bx, bx
	jnz 2f
1:
	mov bx, dx
	test bx, bx
	jnz 2f
	|mov ax, #ECASE
	|push ax
	|jmp .fat
	int 3  | Make the Minix kernel terminate the process with SIGFPE (/bin/sh shows it as: EMT trap).
2:
	jmp (bx)

.globl .csb2
.csb2:
	mov dx, (bx)
	mov cx, 2(bx)
1:
	add bx, #4
	dec cx
	jl 4f
	cmp ax, (bx)
	jnz 1b
	mov bx, 2(bx)
2:
	test bx, bx
	jnz 3f
	|mov ax, #ECASE
	|push ax
	|jmp .fat
	int 3  | Make the Minix kernel terminate the process with SIGFPE (/bin/sh shows it as: EMT trap).
3:
	jmp (bx)
4:
	mov bx, dx
	jmp 2b

.globl .cmi4
.cmi4:
	pop bx  | Return address.
	pop cx
	pop dx
	pop ax
	push si
	mov si, sp
	xchg bx, 2(si)
	pop si
	cmp bx, dx
	jg 1f
	jl 2f
	cmp ax, cx
	ja 1f
	je 3f
2:
	mov ax, #-1
	ret
3:
	xor ax, ax
	ret
1:
	mov ax, #1
	ret

.globl .cmu4
.cmu4:
	pop bx
	pop cx
	pop dx
	pop ax
	push si
	mov si, sp
	xchg bx, 2(si)
	pop si
	cmp bx, dx
	ja 1f
	jb 2f
	cmp ax, cx
	ja 1f
	jz 3f
2:
	mov ax, #-1
	ret
3:
	xor ax, ax
	ret
1:
	mov ax, #1
	ret

| --- C compiler integer operation helpers.

.globl .mli4
.mli4:
| yl=2
| yh=4
	mov bx, sp
	push dx
	mov cx, ax
	mul 4(bx)
	pop dx
	push ax
	mov ax, dx
	mul 2(bx)
	pop dx
	add dx, ax
	mov ax, cx
	mov cx, dx
	mul 2(bx)
	add dx, cx
	pop bx
	add sp, #4
	jmp (bx)

.globl .dvi4
.dvi4:
.define .dvi4
| yl=6
| yh=8
| xl=10
| xh=12
	push si
	push di
	mov si, sp
	mov bx, 6(si)
	mov ax, 8(si)
	cwd
	mov di, dx
	cmp dx, ax
	jnz 7f
	and dx, dx
	jge 1f
	neg bx
	jz 7f
1:
	xor dx, dx
	mov cx, 10(si)
	mov ax, 12(si)
	and ax, ax
	jge 2f
	neg ax
	neg cx
	sbb ax, dx
	not di
2:
	div bx
	xchg ax, cx
	div bx
9:
	and di, di
	jge 1f
	neg cx
	neg ax
	sbb cx, #0
1:
	mov dx, cx
	pop di
	pop si
	pop bx
	add sp, #8
	jmp (bx)
7:
	push dx
	mov di, ax
	xor bx, bx
	and di, di
	jge 1f
	neg di
	neg 6(si)
	sbb di, bx
1:
	mov ax, 10(si)
	mov dx, 12(si)
	and dx, dx
	jge 1f
	neg dx
	neg ax
	sbb dx, bx
	not -2(si)
1:
	mov cx, #16
1:
	shl ax, #1
	rcl dx, #1
	rcl bx, #1
	cmp di, bx
	ja 3f
	jb 2f
	cmp 6(si), dx
	jbe 2f
3:
	loop 1b
	jmp 1f
2:
	sub dx, 6(si)
	sbb bx, di
	inc ax
	loop 1b
1:
	pop di
	jmp 9b

.globl .dvu4
.dvu4:
| yl=6
| yh=8
| xl=10
| xh=12
	push si
	push di
	mov si, sp
	mov bx, 6(si)
	mov ax, 8(si)
	or ax, ax
	jnz 7f
	xor dx, dx
	mov cx, 10(si)
	mov ax, 12(si)
	div bx
	xchg ax, cx
	div bx
9:
	mov dx, cx
	pop di
	pop si
	pop bx
	add sp, #8
	jmp (bx)
7:
	mov di, ax
	xor bx, bx
	mov ax, 10(si)
	mov dx, 12(si)
	mov cx, #16
1:
	shl ax, #1
	rcl dx, #1
	rcl bx, #1
	cmp di, bx
	ja 3f
	jb 2f
	cmp 6(si), dx
	jbe 2f
3:
	loop 1b
	jmp 9b
2:
	sub dx, 6(si)
	sbb bx, di
	inc ax
	loop 1b
	jmp 9b

.globl .rmi4
.rmi4:
| yl=6
| yh=8
| xl=10
| xh=12
	push si
	push di
	mov si, sp
	mov bx, 6(si)
	mov ax, 8(si)
	cwd
	cmp dx, ax
	jnz 7f
	and dx, dx
	jge 1f
	neg bx
	jz 7f
1:
	xor dx, dx
	mov cx, 10(si)
	mov ax, 12(si)
	and ax, ax
	jge 2f
	neg ax
	neg cx
	sbb ax, dx
2:
	div bx
	xchg ax, cx
	div bx
	xor bx, bx
9:
	cmp 12(si), #0
	jge 1f
	neg bx
	neg dx
	sbb bx, #0
1:
	mov ax, dx
	mov dx, bx
	pop di
	pop si
	pop bx
	add sp, #8
	jmp (bx)
7:
	mov di, ax
	xor bx, bx
	and di, di
	jge 1f
	neg di
	neg 6(si)
	sbb di, bx
1:
	mov ax, 10(si)
	mov dx, 12(si)
	and dx, dx
	jge 1f
	neg dx
	neg ax
	sbb dx, bx
1:
	mov cx, #16
1:
	shl ax, #1
	rcl dx, #1
	rcl bx, #1
	cmp di, bx
	ja 3f
	jb 2f
	cmp 6(si), dx
	jbe 2f
3:
	loop 1b
	jmp 9b
2:
	sub dx, 6(si)
	sbb bx, di
	inc ax
	loop 1b
1:
	jmp 9b

.globl .rmu4
.rmu4:
| yl=6
| yh=8
| xl=10
| xh=12
	push si
	push di
	mov si, sp
	mov bx, 6(si)
	mov ax, 8(si)
	or ax, ax
	jnz 7f
1:
	xor dx, dx
	mov cx, 10(si)
	mov ax, 12(si)
2:
	div bx
	xchg ax, cx
	div bx
	xor bx, bx
9:
	mov ax, dx
	mov dx, bx
	pop di
	pop si
	pop bx
	add sp, #8
	jmp (bx)
7:
	mov di, ax
	xor bx, bx
	mov ax, 10(si)
	mov dx, 12(si)
	mov cx, #16
1:
	shl ax, #1
	rcl dx, #1
	rcl bx, #1
	cmp di, bx
	ja 3f
	jb 2f
	cmp 6(si), dx
	jbe 2f
3:
	loop 1b
	jmp 9b
2:
	sub dx, 6(si)
	sbb bx, di
	inc ax
	loop 1b
1:
	jmp 9b

| --- C library string functions (str...(3) and mem...(3)).

| void *memcpy(void *s1, const void *s2, size_t n);
|
|	Copies n characters from the object pointed to by s2 into the
|	object pointed to by s1.  Copying takes place as if the n
|	characters pointed to by s2 are first copied to a temporary
|	area and then copied to the object pointed to by s1.
|	Returns s1.
|
|	Per X3J11, memcpy may have undefined results if the objects
|	overlap; since the performance penalty is insignificant, we
|	use the safe memmove code for it as well.
.globl _memcpy
_memcpy:
	mov bx, si  | Save SI to BX.
	mov dx, di  | Save DI to DX.
	mov di, sp
	mov cx, 6(di)  | Argument n.
	mov si, 4(di)  | Argument s2.
	mov di, 2(di)  | Argument s1.
	mov ax, di  | Save a copy of s1, for returning.
	rep
	movb
	mov di, dx  | Restore DI. DX := junk.
	mov si, bx  | Restore SI. BX := junk.
	ret

| void *memset(void *s, int c, size_t n);
|
|	Copies the value of c (converted to unsigned char) into the
|	first n locations of the object pointed to by s.
|	Returns s.
.globl _memset
_memset:
	mov dx, di  | Save DI to DX.
	mov di, sp
	mov cx, 6(di)  | Argument n.
	movb al, 4(di)  | Argument c.
	mov di, 2(di)  | Argument s.
	mov bx, di  | Save a copy of s, for returning.
	rep
	stob
	xchg ax, bx  | AX := s; BX := junk.
	mov di, dx  | Restore DI. DX is now junk.
	ret

| int strcmp(const char *s1, const char *s2);
|
|	Compares the strings pointed to by s1 and s2.  Returns zero if
|	strings are identical, a positive number if s1 greater than s2,
|	and a negative number otherwise.
.globl _strcmp
_strcmp:
	mov bx, si  | Save SI to BX.
	mov dx, di  | Save DI to DX.
	mov si, sp
	mov di, 4(si)  | Argument s2.
	mov si, 2(si)  | Argument s1.
strcmpnext:
	lodb
	scab
	jne strcmpdiff
	cmpb al, *0
	jnz strcmpnext
	xor ax, ax
	j strcmpdone
strcmpdiff:
	sbb ax, ax
	orb al, *1
strcmpdone:
	mov di, dx  | Restore DI. DX := junk.
	mov si, bx  | Restore SI. BX := junk.
	ret

| char *strcpy(char *s1, const char *s2);
|
|	Copy the string pointed to by s2, including the terminating null
|	character, into the array pointed to by s1.  Returns s1.
.globl _strcpy
_strcpy:
	call strcpysetup
strcpynext:
	lodb
	stob
	testb al, al
	jnz strcpynext
	xchg ax, cx  | AX := s1; CX := junk.
	mov di, dx  | Restore DI. DX := junk.
	mov si, bx  | Restore SI. BX := junk.
	ret
strcpysetup:  | Code shared by _strcpy and _strcat.
	mov bx, si  | Save SI to BX.
	mov dx, di  | Save DI to DX.
	mov di, sp
	mov si, 6(di)  | Argument s2.
	mov di, 4(di)  | Argument s1.
	mov cx, di  | Save a copy of s1, for returning.
	ret

| char *strcat(char *s1, const char *s2)
|
|	Concatenates the string pointed to by s2 onto the end of the
|	string pointed to by s1.  Returns s1.
.globl _strcat
_strcat:
	call strcpysetup
	movb al, *0
strcatnext:
	scab
	jne strcatnext
	dec di  | Undo the skipping over the last NUL.
	j strcpynext

| size_t strlen(const char *s);
|
|	Returns the length of the string pointed to by s.
.globl _strlen
_strlen:
	mov bx, di  | Save DI.
	mov di, sp
	mov di, 2(di)  | Argument s.
	mov cx, #-1
	xorb al, al  | Also sets ZF := 1, which is needed below for the emptry string.
	repne
	scab
	not cx  | Silly trick gives length (including the NUL byte).
	dec cx  | Forget about the NUL byte.
	xchg ax, cx  | AX := result; CX := junk.
	mov di, bx  | Restore DI. BX is now junk.
	ret

| __END__
