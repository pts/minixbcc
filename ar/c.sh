#! /bin/sh --
# by pts@fazekas.hu at Sun Jan 11 20:38:40 CET 2026

set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

gcc -m32 -s -O2 -Wall -W -o ar.cross ar.c

: "$0" OK.
