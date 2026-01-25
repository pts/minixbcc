#!/bin/sh
# by pts@fazekas.hu at Sun Jan 25 00:58:47 CET 2026
test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).
test $# = 0 && set  gcc -s -O2  # Default GCC command and compiler flags.
set -x  # Print all commands run.

test -d bin || mkdir bin || exit "$?"
test -d libexec || mkdir libexec || exit "$?"
"$@" -o bin/bbcc cc/*.c || exit "$?"
"$@" -o libexec/sc sc/*.c || exit "$?"
"$@" -o libexec/as as/*.c || exit "$?"
"$@" -o libexec/ld ld/*.c || exit "$?"
"$@" -o libexec/cr cr/*.c || exit "$?"
"$@" -o libexec/cpp cpp/*.c || exit "$?"

# !! Build the libc (0/libc.a, 3/libc.a) here.

# Quick test 1.
echo '#include <stdio.h>' >h.c || exit "$?"
echo 'int main() { PRINT_F("Hello, World!%c", 10); return 0; }' >>h.c || exit "$?"
rm -f h
bin/bbcc -v -O -UBLAH -D PRINT_F=printf -h9999 -h 8888 -o h h.c || exit "$?"
: ./h || exit "$?"  # We can't run Minix programs.

: "$0" OK.
