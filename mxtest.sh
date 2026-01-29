#!/bin/sh
#
# mxtest: build a hello-world proram using ./sc.mx, ./as.mx and ./ld.mx on Minix
# by pts@fazekas.hu at Thu Jan 29 21:13:16 CET 2026
#

test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).
set -x  # Print all commands run.

sc=./sc.mx
as=./as.mx
ld=./ld.mx
test -f "$sc"
test -x "$sc"
test -f "$as"
test -x "$as"
test -f "$ld"
test -x "$ld"

a03=0  # TODO(pts): Autodetect host.

echo 'int main() { return write(1, "Hello, World!\n", 14) != 14; }' >hw.c
cat hw.c
"$as" -"$a03" -w -o lscs.o sc/lscs"$a03".s || exit "$?"
"$sc" -"$a03" -o hw.s hw.c || exit "$?"
"$as" -"$a03" -u -w -o hw.o hw.s || exit "$?"
"$ld" -"$a03" -o hw lscs.o hw.o || exit "$?"

: "$0" OK.
