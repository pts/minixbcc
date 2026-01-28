#!/bin/sh
#
# install.sh: install files on Minix 1.5.10--2.0.4
# by pts@fazekas.hu at Tue Jan 20 23:37:53 CET 2026
#
# Run this script as `sh install.sh' as root after `sh build.sh' has
# completed successfully.
#

set -x

# Don't copy bin/bbcc, that's the cross-compiler.
cp bin/bbcc.inst /usr/bin/bbcc || exit "$?"  # !! Preserve mtime, everywhere. Preserve mtime when copying with `cp -p'. The Minix 1.5.10 `cp' tool doesn't support `-p'. Build our own cp if needed, or use tar?

libdir=/usr/minixbcc
test -d "$libdir" || mkdir "$libdir" || exit "$?"
# !! Does it succeed when running twice on Minix?
tar cf include.tar include || exit "$?"
(cd "$libdir" && tar xf -) <include.tar || exit "$?"  # !! Does it properly preserve mtime?
rm -f include.tar
for a03 in 0 3; do
  test -d "$libdir/$a03" || mkdir "$libdir/$a03" || exit "$?"
  cp "$a03"/sc "$libdir"/"$a03"/ || exit "$?"
  cp "$a03"/as "$libdir"/"$a03"/ || exit "$?"
  cp "$a03"/ld "$libdir"/"$a03"/ || exit "$?"
  cp "$a03"/cr  "$libdir"/"$a03"/ || exit "$?"  # !! Make this available in /usr/bin.
  cp "$a03"/cpp "$libdir"/"$a03"/ || exit "$?"  # !! Make this available in /usr/bin.
  cp "$a03"/crtso.o "$libdir"/"$a03"/ || exit "$?"
  cp "$a03"/libc.a  "$libdir"/"$a03"/ || exit "$?"
done

# Quick test 1.
echo '#include <stdio.h>' >h.c || exit "$?"
echo 'int main() { PRINT_F("Hello, World!%c", 10); return 0; }' >>h.c || exit "$?"
rm -f h
/usr/bin/bbcc -v -O -UBLAH -D PRINT_F=printf -h9999 -h 8888 -o h h.c || exit "$?"
./h || exit "$?"

# Quick test 2.
/usr/bin/bbcc as -0 -a -w -o 0/lscs.o sc/lscs0.s || exit "$?"
if test -f 0g/lscs.n; then
  cmp 0g/lscs.n 0/lscs.o || exit "$?"
fi
rm -f 0/lscs.o
/usr/bin/bbcc -v as -0 -a -w -o 0/lscs.o sc/lscs0.s || exit "$?"
if test -f 0g/lscs.n; then
  cmp 0g/lscs.n 0/lscs.o || exit "$?"
fi

: "$0" OK.
