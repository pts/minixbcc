#! /bin/sh --
# by pts@fazekas.hu at Thu Jan  8 14:49:17 CET 2026

set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

# -DS_ALIGNMENT=1 would be safe on x86, but not elsewhere.
gcc -m32 -s -O2 -Dunix -Wall -W -Wno-char-subscripts -o ld.cross dumps.c io.c ld.c readobj.c table.c typeconv.c writebin.c

: "$0" OK.
