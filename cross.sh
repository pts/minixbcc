#!/bin/sh
# by pts@fazekas.hu at Sun Jan 25 00:58:47 CET 2026
test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).
set -x  # Print all commands run.

# --- Build the cross-compiler tools.

emu=
test "$1" = --emu && test "$2" && emu="$2" && shift && shift
if test "$1" = --libc; then  # Called by build.sh to build the libc.
  sc="$2"; as="$3"; cr="$4"; cmp="$5"
  test -f "$cr" || exit "$?"
  test -x "$cr" || exit "$?"
else  # Regular (interactive) operation: build the cross-compiler tools.
  cmp=:; test "$1" = --cmp && cmp=cmp && shift  # Specify --cmp to compare libc files against golden files.
  test $# = 0 && set  gcc -s -O2  # Default GCC command and compiler flags.
  test -d bin || mkdir bin || exit "$?"
  test -d libexec || mkdir libexec || exit "$?"
  "$@" -o bin/bbcc cc/*.c || exit "$?"
  "$@" -o libexec/sc sc/*.c || exit "$?"
  "$@" -o libexec/as as/*.c || exit "$?"
  "$@" -o libexec/ld ld/*.c || exit "$?"
  "$@" -o libexec/cr cr/*.c || exit "$?"
  "$@" -o libexec/cpp cpp/*.c || exit "$?"
  sc=libexec/sc; as=libexec/as; cr=libexec/cr  # Tools to run to build the libc below.
  set x cc; shift
fi

# --- Build the target libc object files for each system.
#
# We don't need the target tools for this, because we use the host tools ("$sc", "$as").

# Relevant flags of the assembler ($as):
# -0  start with 16-bit code segment; default: native
# -3  start with 32-bit code segment; default: native
# -a  enable partial source compatibility with Minix 1.5.10 asld
# -g  only put global symbols in object file  # !! Start using it.
# -n name.s  module name (without extension) to put to .o file
# -o name.o  produce object file
# -u  take undefined symbols as imported-with-unspecified segment
# -w  don't print warnings

ANSIB='abort abs assert atoi atol bsearch ctime ctype errno exit fclose fflush fgetc fgets fopen fprintf fputc fputs fread freopen fseek ftell fwrite getenv gets malloc perror puts qsort rand scanf setbuf signal sprintf strerror strtol strtoul system time tmpnam ungetc vsprintf'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/ansi . Patched by Bruce Evans.
IBMB='peekpoke portio'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/ibm . Patched by Bruce Evans.
OTHERB='amoeba bcmp bcopy brk bzero call chroot cleanup crypt curses doprintf ffs getdents getopt getpass gtty index ioctl itoa lock lrand lsearch memccpy message mknod mktemp mount nlist popen printdat printk prints ptrace putenv regexp regsub rindex seekdir stb stderr stime stty swab sync syslib telldir termcap umount uniqport vectab'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/other . Patched by Bruce Evans.
POSIXB='_exit access alarm chdir chmod chown close closedir creat ctermid cuserid dup dup2 exec execlp fcntl fdopen fork fpathconf fstat getcwd getegid geteuid getgid getgrent getlogin getpid getppid getpwent getuid isatty kill link lseek mkdir mkfifo open opendir pathconf pause pipe read readdir rename rewinddir rmdir setgid setuid sleep stat sysconf times ttyname umask unlink utime wait write'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/posix . Patched by Bruce Evans.
SB='crtso head'  # Minix startup routines, assembly source files (*.s) by Bruce Evans. !! Should head.s be removed from libc? A file named like this seems to be used for the Minix kernel, but doesn't it have its own copy?
IB='idiv idivu imod imodu imul isl isr isru'  # BCC compiler support for integer arithmetic, assembly source files (*.s) by Bruce Evans.
MB='brksize catchsig sendrec setjmp'  # Minix library routines, assembly source files (*.s) by Bruce Evans.
STRMASMB='memcmp memmove memset strcmp strcpy strlen strncmp strncpy'  # Minix library string routines, assembly source files (*.s) with i86 code based on to-be-preprecessed assembly source files (*.x) from Minix 1.5.10 /usr/src/lib/string, and i386 code copied from inline assembly in C source files (*.c) by Bruce Evans.
LLB0='laddl landl lcmpl lcoml ldecl ldivl ldivul leorl lincl lmodl lmodul lmull lnegl lorl lsll lsrl lsrul lsubl ltstl ldivmod'  # BCC compiler support for long arithmetic on little-endian (x86) longs, i86 assembly source files (*.s) by Bruce Evans.
STRB03='memchr strcat strncat strchr strcoll strrchr strspn strstr strtok strxfrm strpbrk strcspn'  # Minix library string routines, assembly source files (*.s) created manually from to-be-preprecessed assembly source files (*.x) from Minix 1.5.10 /usr/src/lib/string . Also: Minix library string routines, C source files (*.c) without assembly from Minix 1.5.10 /usr/src/lib/string . Both patched by Bruce Evans.

for a03 in 0 3; do
  case "$a03" in
   0) LLB="$LLB0"; STRASMB="$STRB03"; STRCB= ;;  # i86.
   3) LLB=; STRASMB=; STRCB="$STRB03" ;;  # i386.
  esac
  test -d "$a03" || mkdir "$a03" || exit "$?"
  for b in $STRMASMB $STRASMB $SB $IB $JB $LBB $LLB $MB; do
    rm -f "$a03"/"$b".o  # !!
    $emu "$as" -"$a03" -w $aflag -o "$a03"/"$b".o libc/"$b".s || exit "$?"
    "$cmp" "$a03"/"$b".o "$a03"g/"$b".n || exit "$?"  # Compare object file (*.o) to golden file (*.n).
  done
  for b in $STRCB $ANSIB $IBMB $OTHERB $POSIXB; do
    # bcc -v -"$a03" -c -O -D_MINIX -D_POSIX_SOURCE -o "$a03"/"$b".o "$b".c
    rm -f "$a03"/"$b".o "$a03"/"$b".s  # !!
    $emu "$sc" -"$a03" -Iinclude -D_MINIX -D_POSIX_SOURCE -o "$a03"/"$b".s libc/"$b".c || exit "$?"
    $emu "$as" -"$a03" -u -w -o "$a03"/"$b".o "$a03"/"$b".s || exit "$?"
    "$cmp" "$a03"/"$b".o "$a03"g/"$b".n || exit "$?"  # Compare object file (*.o) to the relavant v3 golden file (*.m).
    rm -f "$a03"/"$b".s || exit "$?"
  done
done

# --- Build the target libc library archive (*.a) files for each system.
#
# We don't need the target tools for this, because we use the host tools ("$cr").

cru="$cr"; case "$cr" in /*) ;; */*) cru="../$cr" ;; esac  # "$cru" is "$cr", but it works from one directory lower.
emuu="$emu"; case "$emu" in /*) ;; */*) emuu="../$emu" ;; esac  # "$emuu" is "$emu", but it works from one directory lower.
# !! Increment typestamp because of changes in isatty.c etc.
# !! Redo both in alphabetic order.
(cd 0 && $emuu "$cru" -o libc.a -t @644973285 _exit.o abort.o abs.o access.o alarm.o amoeba.o assert.o atoi.o atol.o bcmp.o bcopy.o brk.o brksize.o bsearch.o bzero.o call.o catchsig.o chdir.o chmod.o chown.o chroot.o cleanup.o close.o closedir.o creat.o crypt.o ctermid.o ctime.o ctype.o curses.o cuserid.o doprintf.o dup.o dup2.o errno.o exec.o execlp.o exit.o fclose.o fcntl.o fdopen.o fflush.o ffs.o fgetc.o fgets.o fopen.o fork.o fpathconf.o fprintf.o fputc.o fputs.o fread.o freopen.o fseek.o fstat.o ftell.o fwrite.o getcwd.o getdents.o getegid.o getenv.o geteuid.o getgid.o getgrent.o getlogin.o getopt.o getpass.o getpid.o getppid.o getpwent.o gets.o getuid.o gtty.o idiv.o idivu.o imod.o imodu.o imul.o index.o ioctl.o isatty.o isl.o isr.o isru.o itoa.o kill.o laddl.o landl.o lcmpl.o lcoml.o ldecl.o ldivl.o ldivmod.o ldivul.o leorl.o lincl.o link.o lmodl.o lmodul.o lmull.o lnegl.o lock.o lorl.o lrand.o lsearch.o lseek.o lsll.o lsrl.o lsrul.o lsubl.o ltstl.o malloc.o memccpy.o memchr.o memcmp.o memmove.o memset.o message.o mkdir.o mkfifo.o mknod.o mktemp.o mount.o nlist.o open.o opendir.o pathconf.o pause.o peekpoke.o perror.o pipe.o popen.o portio.o printdat.o printk.o prints.o ptrace.o putenv.o puts.o qsort.o rand.o read.o readdir.o regexp.o regsub.o rename.o rewinddir.o rindex.o rmdir.o scanf.o seekdir.o sendrec.o setbuf.o setgid.o setjmp.o setuid.o signal.o sleep.o sprintf.o stat.o stb.o stderr.o stime.o strcat.o strchr.o strcmp.o strcoll.o strcpy.o strcspn.o strerror.o strlen.o strncat.o strncmp.o strncpy.o strpbrk.o strrchr.o strspn.o strstr.o strtok.o strtol.o strtoul.o strxfrm.o stty.o swab.o sync.o sysconf.o syslib.o system.o telldir.o termcap.o time.o times.o tmpnam.o ttyname.o umask.o umount.o ungetc.o uniqport.o unlink.o utime.o vectab.o vsprintf.o wait.o write.o) || exit "$?"
"$cmp" 0/libc.a 0g/libc.d || exit "$?"
(cd 3 && $emuu "$cru" -o libc.a -t @645451200 abort.o abs.o assert.o atoi.o atol.o bsearch.o ctime.o ctype.o errno.o exit.o fclose.o fflush.o fgetc.o fgets.o fopen.o fprintf.o fputc.o fputs.o fread.o freopen.o fseek.o ftell.o fwrite.o getenv.o gets.o malloc.o memchr.o memmove.o perror.o puts.o qsort.o rand.o scanf.o setbuf.o signal.o sprintf.o strcat.o strchr.o strcoll.o strerror.o strrchr.o strspn.o strstr.o strtok.o strtol.o strtoul.o strxfrm.o system.o time.o tmpnam.o ungetc.o vsprintf.o peekpoke.o portio.o amoeba.o bcmp.o bcopy.o brk.o bzero.o call.o chroot.o cleanup.o crypt.o curses.o doprintf.o ffs.o getdents.o getopt.o getpass.o gtty.o index.o ioctl.o itoa.o lock.o lrand.o lsearch.o memccpy.o message.o mknod.o mktemp.o mount.o nlist.o popen.o printdat.o printk.o prints.o ptrace.o putenv.o regexp.o regsub.o rindex.o seekdir.o stb.o stderr.o stime.o stty.o swab.o sync.o syslib.o telldir.o termcap.o umount.o uniqport.o vectab.o _exit.o access.o alarm.o chdir.o chmod.o chown.o close.o closedir.o creat.o ctermid.o cuserid.o dup.o dup2.o exec.o execlp.o fcntl.o fdopen.o fork.o fpathconf.o fstat.o getcwd.o getegid.o geteuid.o getgid.o getgrent.o getlogin.o getpid.o getppid.o getpwent.o getuid.o isatty.o kill.o link.o lseek.o mkdir.o mkfifo.o open.o opendir.o pathconf.o pause.o pipe.o read.o readdir.o rename.o rewinddir.o rmdir.o setgid.o setuid.o sleep.o stat.o sysconf.o times.o ttyname.o umask.o unlink.o utime.o wait.o write.o memcmp.o memset.o strcmp.o strcpy.o strcspn.o strlen.o strncat.o strncmp.o strncpy.o strpbrk.o idiv.o idivu.o imod.o imodu.o imul.o isl.o isr.o isru.o brksize.o catchsig.o sendrec.o setjmp.o) || exit "$?"
"$cmp" 3/libc.a 3g/libc.d || exit "$?"
test "$1" = --libc && exit

# --- Do some tests.

# Quick test 1.
echo '#include <stdio.h>' >h.c || exit "$?"
echo 'int main() { PRINT_F("Hello, World!%c", 10); return 0; }' >>h.c || exit "$?"
rm -f h
bin/bbcc -v -O -UBLAH -D PRINT_F=printf -h9999 -h 8888 -o h h.c || exit "$?"
: ./h || exit "$?"  # We can't run Minix programs.

# Quick test 2.
bin/bbcc as -0 -a -w -o 0/lscs.o sc/lscs0.s || exit "$?"
cmp 0g/lscs.n 0/lscs.o || exit "$?"
rm -f 0/lscs.o
bin/bbcc -v as -0 -a -w -o 0/lscs.o sc/lscs0.s || exit "$?"
cmp 0g/lscs.n 0/lscs.o || exit "$?"

: "$0" OK.
