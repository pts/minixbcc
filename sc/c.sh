#! /bin/sh --
# by pts@fazekas.hu at Thu Jan  8 14:49:17 CET 2026

set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

gcc -m32 -s -O2 -Wall -W -Wno-maybe-uninitialized -Wno-parentheses -Wno-char-subscripts -Wno-missing-field-initializers -Wno-sign-compare -fno-strict-aliasing \
    -DDEFAULT_INCLUDE_DIR='"../include"' \
    -o bcc3-cc1g bcc-cc1.c assign.c codefrag.c debug.c declare.c express.c exptree.c floatop.c function.c gencode.c genloads.c glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c softop.c state.c table.c type.c
rm -f ./*.s
for f in bcc-cc1.c assign.c codefrag.c debug.c declare.c express.c exptree.c floatop.c function.c gencode.c genloads.c glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c softop.c state.c table.c type.c; do
  ./bcc3-cc1g -3 -o "${f%.*}".s "$f"
done
if grep -E 'call.([fF]|dto|_atof|_strto(f|ld|ld))' ./*.s; then
  : 'Floating point operation detected in C compiler.'
  false
fi

: "$0" OK.
