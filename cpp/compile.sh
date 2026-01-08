#! /bin/sh --
# by pts@fazekas.hu at Thu Jan  8 23:03:03 CET 2026
set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

gcc -m32 -s -O2 -DOLD_PREPROCESSOR -DFILE_LOCAL=static -Dunix -Wall -W -o cpp cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c
ls -l cpp

: "$0" OK.
