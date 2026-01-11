#! /bin/sh --
# by pts@fazekas.hu at Sun Jan 11 20:38:40 CET 2026

set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

gcc -m32 -s -O2 -Wall -W -DMINIX_SYNTAX -o as.cross as.c assemble.c error.c express.c genbin.c genlist.c genobj.c gensym.c keywords.c macro.c mops.c pops.c readsrc.c scan.c table.c typeconv.c

: "$0" OK.
