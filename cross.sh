#!/bin/sh
# by pts@fazekas.hu at Sun Jan 25 00:58:47 CET 2026
test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).
test $# = 0 && set  gcc -s -O2  # Default GCC command and compiler flags.
set -x  # Print all commands run.

"$@" -o sc.cross sc/*.c || exit "$?"
"$@" -o as.cross as/*.c || exit "$?"
"$@" -o ld.cross ld/*.c || exit "$?"
"$@" -o cr.cross cr/*.c || exit "$?"
"$@" -o cpp.cross cpp/*.c || exit "$?"

# !! Build the libc (0/libc.a, 3/libc.a) here.

: "$0" OK.
