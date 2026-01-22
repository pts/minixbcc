#!/bin/sh
#
# install.sh: install files on Minix 1.5.10--2.0.4
# by pts@fazekas.hu at Tue Jan 20 23:37:53 CET 2026
#
# Run this script as `sh install.sh' as root after `sh build.sh' has
# completed successfully.
#

set -x

bcc -O -i -o bbcc.host bbcc/bbcc.c || exit "$?" # !! move this to build.sh, don't use bcc yet. !! Build for all targets.
# !! Preserve mtime with tar.
cp bbcc.host /usr/bin/bbcc || exit "$?"
libdir=/usr/minixbcc
test -d "$libdir" || mkdir "$libdir" || exit "$?"
tar cf include.tar include || exit "$?"
(cd "$libdir" && tar xf -) <include.tar || exit "$?"  # !! Does it properly copy mtime?
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

# Quick test.
echo '#include <stdio.h>' >h.c || exit "$?"
echo 'int main() { printf("Hello, World!\n"); return 0; }' >>h.c || exit "$?"
rm -f h
/usr/bin/bbcc -v -O -o h h.c || exit "$?"
./h || exit "$?"

: "$0" OK.
