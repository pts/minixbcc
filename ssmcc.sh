#!/bin/sh
#
# ssmcc.sh: cross-compiles some of the tools to Minix i86 using https://github.com/pts/ssmcc (OpenWatcom v2 C compiler) on Linux
# by pts@fazekas.hu at Thu Jan 29 20:38:41 CET 2026
#

test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).
set -x  # Print all commands run.

ssmcc=ssmcc/ssmcc

ssmccu="$ssmcc"; case "$ssmcc" in /*) ;; */*) ssmccu="../$ssmcc" ;; esac  # "$ssmccu" is "$ssmcc", but it works from one directory lower.
(cd sc && "$ssmccu" -bminixi86 -Os -W -Wall -o ../sc.mx bcc-cc1.c assign.c codefrag.c debug.c declare.c express.c exptree.c floatop.c function.c gencode.c genloads.c glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c softop.c state.c table.c type.c) || exit "$?"
(cd as && "$ssmccu" -bminixi86 -Os -W -Wall -o ../as.mx as.c assemble.c error.c express.c genbin.c genlist.c genobj.c gensym.c heap.c keywords.c macro.c mops.c pops.c readsrc.c scan.c table.c typeconv.c) || exit "$?"
(cd ld && "$ssmccu" -bminixi86 -Os -W -Wall -o ../ld.mx dumps.c heap.c io.c ld.c readobj.c table.c typeconv.c writebin.c) || exit "$?"
(cd cr && "$ssmccu" -bminixi86 -Os -W -Wall -o ../cr.mx cr.c) || exit "$?"
#(cd cr && "$ssmccu" -bminixi86 -Os -W -Wall -o ../cc.mx cr.c) || exit "$?"  # There is not enough libc support for this yet.
#(cd cpp && "$ssmccu" -bminixi86 -Os -W -Wall -o ../cpp.mx cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c) || exit "$?"  # There is not enough libc support for this yet.

: "$0" OK.
