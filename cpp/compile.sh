#! /bin/sh --
# by pts@fazekas.hu at Thu Jan  8 23:03:03 CET 2026
set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

gcc -s -O2 -DOLD_PREPROCESSOR -DFILE_LOCAL=static -Dunix -Wall -W -o cpp cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c
ls -l cpp
gcc -m32 -s -O2 -DOLD_PREPROCESSOR -DFILE_LOCAL=static -Dunix -Wall -W -o cpp.m32 cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c
gcc -m64 -s -O2 -DOLD_PREPROCESSOR -DFILE_LOCAL=static -Dunix -Wall -W -o cpp.m64 cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c
minicc --gcc=4.8 -Werror -DOLD_PREPROCESSOR -DFILE_LOCAL=static -Dunix -o cpp.minigcc cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c
minicc -Werror -Wno-n308 -Wno-n309 -DOLD_PREPROCESSOR -DFILE_LOCAL=static -Dunix -o cpp.miniwcc cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c

: "$0" OK.
