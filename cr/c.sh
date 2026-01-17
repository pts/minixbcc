#! /bin/sh --
# by pts@fazekas.hu at Thu Jan  8 14:49:17 CET 2026

set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

gcc -m32 -s -O2 -Wall -W -o cr.cross cr.c

: "$0" OK.
