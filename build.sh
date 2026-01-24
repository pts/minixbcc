#!/bin/sh
#
# build.sh: build the BCC compiler tools, libc and extr tools
# by pts@fazekas.hu at Thu Jan  8 15:51:45 CET 2026
#
# !! make it work with `minicc --gcc=4.2' (missing memcpy), `minicc --pcc' (missing prototypes for cpp; code generation difference in sc), `minicc --tcc' (missing memcpy) and `minicc --utcc' (ld.cross segfaults)
# !! add driver tool command-line
# !! add driver flag -E to invoke CPP
# !! test isatty implementations, espacially the assembly implementation in sc
# !! for some systems, set S_ALIGNMENT=1 in ld/config.h
# !! doc: Minix 1.7.0 has the ANSI C compiler (ncc) as /usr/bin/cc  ; make it work
# !! (after porting to ack) Remove the `t' symbols from `nm as.mx' etc. added by the assembler; use symbols starting with . instead?
# !! Make the `bcc -i` and `ld -i` (separate I&D) default independent of the BCC driver version.
# !! Port to -m64.
# !! Port to big-endian architecture.
# !! Port to sizeof(long) == 8.
# !! Add patch to /usr/include to add `#ifndef __BCC__' for functions returning a struct.
# !! Convert .x to .s, no need for preprocessor.
# !! Is the `j' jump output of bcc3 compatible with as0? (as0 expects jmp as short jump.)
# !! Port isatty.c to Minix 2.0.4 (from Minix 1.5.10).
# !! Add the remaining patches to cpp.
# !! strtol -1 / 2 sign incompatibility with the i86 /local/bin/sc (== -1); the other one returns 0.
# !! replace divisions with right shifts (BCC is not smart enough to optimize it, it also means something different)
# !! fix sar code generation bug in bcc3 for div2(...), div4(...) etc. in bcc3
# !! fix suboptimal i386 code generation in ld for `v += (unsigned long) (unsigned char) c;'; this already uses movzx: `v += (unsigned char) c;'
# !! everywhere (sc, as, ld, cpp)
#    ifdef ACKFIX  /* Workaround for the bug in Minix 1.5.10 i86 ACK 3.1 C compiler, which sign-extends the (unsigned char) cast. */
#    #  define UCHARCAST(c) (unsigned char) ((unsigned) (c) & 0xff)
#    #else
#    #  define UCHARCAST(c) ((unsigned char) (c))
#    #endif
# !! make sure that constant folding division is always correct in sc: it is on C99 (but not on C89), and it is on x86 (__i386__, __i386__, i8086, i8088, i80386 etc.), otherwise use mininasm implementation:
#    /*
#     ** Deterministic signed division, rounds towards zero.
#     ** The result is undefined if b == 0. It's defined for a == int_min and b == -1.
#     */
#    static value_t value_div(value_t a, value_t b) {
#        const char an = (a < 0);
#        const char bn = (b < 0);
#        const uvalue_t d = (uvalue_t)(an ? -a : a) / (uvalue_t)(bn ? -b : b);
#        return an == bn ? d : -d;
#    }
#

test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).
# test "$0" = "${0%/*}" || cd "${0%/*}" || exit "$?"

if test x"$1" = x--overwrite-golden; then
  cmp=cp; diff=cp; tcmp="$cmp"; tdiff="$diff"; shift  # Overwrite golden files (*r, *.n, *.d). This is dangerous.
  exit 5  # !! Dangerous.
elif test x"$1" = x--nocmp; then
  cmp=true; diff=true; tcmp="$cmp"; tdiff="$diff"; shift  # Ignore golden files.
elif test x"$1" = x--notoolscmp; then
  cmp=cmp; diff=diff; tcmp=true; tdiff=true; shift  # Ignore golden files for tools, compare generated files to non-tools golden files.
elif test x"$1" = x--toolsecho; then  # This works with Linux /bin/sh, but not with Minix /bin/sh.
  cmp=true; diff=true; tcmp=tcmp; tdiff=tdiff; shift  # Compare generated files to non-tools golden files.
  : >tools.diff || exit "$?"
elif test x"$1" = x--toolscp; then  # This works with Linux /bin/sh, but not with Minix /bin/sh.
  cmp=true; diff=true; tcmp=cp; tdiff=cp; shift  # Compare generated files to non-tools golden files. Overwrite golden files (*.r, *.n, *.d) for tools, which is dangerous.
  : >tools.diff || exit "$?"
elif test x"$1" = x--cmp; then
  cmp=cmp; diff=diff; tcmp="$cmp"; tdiff="$diff"; shift  # Compare generated files to golden files.
else
  cmp=cmp; diff=diff; tcmp="$cmp"; tdiff="$diff"   # !! Change this default to `cmp=true; diff=true' later.
fi

set -x  # Echo all commands run.
h=  # Host system unnown so far. We also use `test "$h"' below to check if we've recognized "$1".

for a03 in 0 3 bin; do
  if test -d "$a03"; then :; else
    mkdir "$a03" || exit "$?"
    test -d "$a03" || exit "$?"
  fi
done

# --- Build the host tools or cross-compilation tools: "$sc", "$as", "$ld", "$cr".

if test x"$1" = x"" || test "$1" = auto; then  # Autodetect the system and the C compiler.
  test "$#" = 0 || shift
  rm -f sysftype
  test -f sysftype.c || exit "$?"
  cc -O -o sysftype sysftype.c || exit "$?"
  h="`./sysftype ./sysftype`"
  if test "$h" = 0; then  # Minix i86.
    if bcc -O -o sysftype sysftype.c && test "`./sysftype ./sysftype`" = 3; then
      set bcc0 "$@"
    else
      set dcc0 "$@"
    fi
  elif test "$h" = 3; then  # Minix i386.
    if bcc -O -o sysftype sysftype.c && test "`./sysftype ./sysftype`" = 3; then
      set bcc3 "$@"
    else
      set dcc3 "$@"
    fi
  else
    h=cross  # Host system is a cross-compiler. Other values: h=0 means Minix i86; h=3 means Minix i386.
    rm -f t2
    if gcc -s -O2 -Wall -W -Dfoo -o t2 sysftype.c && ./t2 t2; then
      set gcc "$@"
    else
      set cc "$@"
    fi
    rm -f t2
  fi
  h=; rm -f sysftype
fi

# !! Add support for mxbcc, mxbcc0 and mxbcc3.
# !! Add support for -nostdlib in the mxbcc driver.
if test "$1" = bcc; then  # Autodetect the system for bcc.
  test "$#" = 0 || shift
  rm -f sysftype
  cc -O -o sysftype sysftype.c || exit "$?"
  h="`./sysftype ./sysftype`"
  if test "$h" = 0; then set bcc0 "$@"  # Minix i86.
  elif test "$h" = 3; then set bcc3 "$@"  # Minix i386.
  else
    : "fatal: Minix host system needed for bcc"
    exit 1
  fi
  h=; rm -f sysftype
fi

if test "$1" = dcc0 || test "$1" = dcc3; then  # On Minix i86 or i386, autodetect whether cc is the ACK C compiler, the ACK ANSI C compiler, BCC or anything else.
  cc=cc
  # Doing it in a subshell to avoid `argument list too long' errors later. This may be a bug in Minix 1.5.10 /bin/sh.
  ( echo "int main() { return 0; }" >t.c || exit "$?"  # !! Avoid warning from the ANSI C compiler: 'main' old-fashioned function definition
    m=cc; rm -f t.out  # The default is using cc as a cross-compiler.
    "$cc" -v -o t t.c 2>&1 | (while read acmd arest; do  # Use `-v' to see which commands cc runs.
      case "$acmd" in
       */asld) test "$1" = dcc0 && m=ack0 ;;
       */ncpp | *.ansi) if test "$1" = dcc0; then m=acka0; else m=acka3; fi ;;  # In Minix 1.7.0 i386, the preprocessor is /usr/lib/em_cemcom.ansi.
       */sc | */bcc-*) if test "$1" = dcc0; then m=bcc0; else m=bcc3; fi ;;
      esac
    done; echo "$m" >t.out))
  m="`cat t.out`" || exit "$?"
  rm -f t.c t.out
  test "$m" || exit 2
  shift; set "$m" "$@"
fi

if test "$1" = gcc || test "$1" = clang || test "$1" = owcc || test "$1" = minicc || test "$1" = cc; then  # For cross-compiling with GCC (gcc) or Clang (clang) (e.g. on Linux, FreeBSD, macOS), OpenWatcom v2 (owcc) on Linux, minilibc686 minicc on Linux, or a generic Unix C compiler (cc) on Unix.
  # GCC is known to work with GCC 4.3--4.9 and GCC 7.5.0.
  # Clang is known to work with Clang 6.0.0.
  # Example invocation for OpenWatcom v2 on Linux: ./build.sh owcc
  # Example strict mode invocations:
  # * GCC on modern Unix-like systems: .  /build.sh gcc   -s -O2 -DDEBUG_SIZE_NOPAD -W -Wall -Werror -Wstrict-prototypes -Wno-maybe-uninitialized
  # * Clang on modern Unix-like systems: ./build.sh clang -s -O2 -DDEBUG_SIZE_NOPAD -W -Wall -Werror -Wstrict-prototypes -Wno-maybe-uninitialized -Wno-unknown-warning-option
  # * OpenWatcom v2 on Linux:            ./build.sh owcc  -s -O2 -DDEBUG_SIZE_NOPAD -W -Wall -Werror
  # * minilibc386 and OpenWatcom v2:     ./build.sh minicc -DDEBUG_SIZE_NOPAD -Werror
  # * minilibc386 and GCC 4.8:           ./build.sh minicc --gcc=4.8 -DDEBUG_SIZE_NOPAD -Werror
  # minicc is from http://github.com/pts/minilibc686
  # Assumptions about the host system:
  # * .text can be 75 KiB. This doesn't hold for ELKS and Minix i86.
  # * There is 140 KiB of virtual memory for each process (total of: .text, .rodata, .data, .bss, stack). This doesn't hold for ELKS and Minix i86.
  # * malloc(...) can allocate 192 KiB on top of that. This doesn't hold for ELKS and Minix i86.
  # * There is no need to declare the maximum memory use of a program (including the use of malloc(...)) at compile time. This doesn't hold for ELKS, Minix i86 and Minix i386. For these system, chmem (or `ld -h ...') has to be used. !! Autodetect this.
  # !! make it work with: gcc -ansi -pedantic
  # !! make new enough GCC and Clang work without sysdet, e.g. with __SIZEOF_INT__, __SIZEOF_LONG__, __UINTPTR_TYPE__, __i386__ or __code_model_small__ etc. for PORTALIGN
  rm -f sysdet
  cc="$1"; cflags=-O; shift
  test "$cc" = minicc && cflags=  # It optimizes for size better by default than with -O.
  test "$1" = -O0 && shift && cflags=  # Cancel the -O, in case the C compiler doesn't support it.
  "$cc" $cflags "$@" -o sysdet sysdet.c || exit "$?"
  sysdet="`./sysdet ./sysdet`"  # Typically: sysdet="-DINT32T=int -DINTPTRT=int -DALIGNBYTES=4 -DPORTALIGN"  # !! Add -DMINALIGNBYTES=1
  test "$?" = 0 || exit 2
  rm -f sysdet
  case "$sysdet" in *-DBAD* | "") exit 3 ;; *-DINTPTRT=*) ;; *) exit 4 ;; esac
  # !! Autodetect the -DACKFIX etc. flags in $sysdet in case acka3 is used as cc.

  "$cc" $cflags $sysdet "$@" -o sc.cross sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
  "$cc" $cflags $sysdet "$@" -DMINIX_SYNTAX -o as.cross as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
  "$cc" $cflags $sysdet "$@" -DDEBUG_SIZE_NOPAD -o ld.cross ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
  "$cc" $cflags $sysdet "$@" -o cr.cross cr/cr.c || exit "$?"
  "$cc" $cflags "$@" -DOLD_PREPROCESSOR -Dunix -DHOST=1 -DTARGET=0 -DMACHINE=\"i8088\" -DSYSTEM=\"minix\" -DCOMPILER=\"__STD_CC__\" -o cpp.cross cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"

  sc=./sc.cross
  as=./as.cross
  ld=./ld.cross
  cr=./cr.cross
  h=cross  # Host system is a cross-compiler. Other values: h=0 means Minix i86; h=3 means Minix i386.
fi

if test "$1" = localbin || test "$1" = bccbin; then  # Use /local/bin/{sc,as,ld} v0 from BCC v0 bccbin16.tar.Z or bccbin32.tar.Z on Minix to build the tools.
  # !! Also add support for "$1" = bccbin16 or bccbin32 etc.
  if test "$1" = localbin; then
    sc=/local/bin/sc
    as=/local/bin/as
    ld=/local/bin/ld
    rm -f true0 true0.o true3 true3.o
    "$as" -3 -o true3.o true3.s || exit "$?"
    "$ld" -3 -i -o true3 true3.o || exit "$?"
    test -x true3 || exit "$?"
    if ./true3; then  # Minix i386.
      h=3r  # Host system is Minix i386.
    else
      "$as" -0 -o true0.o true0.s || exit "$?"
      "$ld" -0 -i -o true0 true0.o || exit "$?"
      ./true0 || exit "$?"  # Neither Minix i86 nor Minix i386 detected.
      h=0r  # Host system is Minix i86.
    fi
  elif test -f bccbin32/sc && test -x bccbin32/sc && bccbin32/sc </dev/null >/dev/null; then
    sc=bccbin32/sc
    as=bccbin32/as
    ld=bccbin32/ld
    h=3r
  elif test -f bccbin16/sc && test -x bccbin16/sc && bccbin16/sc </dev/null >/dev/null; then
    sc=bccbin16/sc
    as=bccbin16/as
    ld=bccbin16/ld
    h=0r
  else
    : "fatal: bccbin tools not found"; exit 2
  fi
  cr=  # Will be set later.

  if test "$h" = 3r; then
    sh toolsb.sh 3 "$sc" "$as" "$ld" "" true true i || exit "$?"
  else
    cp ld/globvar.h ld/globvar  # sc v0 i86 (but not i386) has a bug: it can't find #include files with basenames this long when included from ld/io.c. We work it around by using a copy with -DGLOBVARI.
    sh toolsb.sh 0 "$sc" "$as" "$ld" "-DGLOBVARI" true true i || exit "$?"
  fi
  mv sc.tool sc.mx || exit "$?"
  mv as.tool as.mx || exit "$?"
  mv ld.tool ld.mx || exit "$?"
  rm -f cr.tool
  sc=./sc.mx
  as=./as.mx
  ld=./ld.mx
fi

if test "$1" = ack0; then  # Minix 1.5.10 i86 ACK 3.1 C compiler with either the old (1990-06-01) or the new asld (1994-03-29).
  cflags="-DSMALLMEM -DINT32T=long -DINTPTRT=int -DALIGNBYTES=4"
  rm -f sc.mx as.mx ld.mx cr.mx cpp.mx
  # -i for separate I&D (BCC has it by default)
  # -s for adding symbols (opposite meaning as in BCC bcc and GCC gcc)
  cc -i    -O $cflags -DOPEN00 -DNOUNIONINIT -DACKFIX -DSBRK -DLIBCH -c sc/mxmalloc.c sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
  # This old asld (but not the new asld) would run out of memory if we used the Minix 1.5.10 libc (/usr/lib/crtso.c ... /usr/lib/libc.a /usr/lib/end.s).
  # We work it around by using a small, custom libc tailored for sc (lsca.s mxmalloc.c ... lsce.s).
  asld -i -o sc.mx sc/lsca.s mxmalloc.s bcc-cc1.s assign.s codefrag.s debug.s declare.s express.s exptree.s floatop.s function.s gencode.s genloads.s glogcode.s hardop.s input.s label.s loadexp.s longop.s output.s preproc.s preserve.s scan.s softop.s state.s table.s type.s sc/lsce.s || exit "$?"
  rm -f mxmalloc.s bcc-cc1.s assign.s codefrag.s debug.s declare.s express.s exptree.s floatop.s function.s gencode.s genloads.s glogcode.s hardop.s input.s label.s loadexp.s longop.s output.s preproc.s preserve.s scan.s softop.s state.s table.s type.s
  cc -i -s -O $cflags -DMINIX_SYNTAX -DMINIXHEAP -DBRKSIZE -o as.mx as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
  cc -i -s -O $cflags -DMINIXHEAP -DBRKSIZE -DACKFIX -o ld.mx ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
  cc -i -s -O $cflags -o cr.mx cr/cr.c || exit "$?"
  # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
  cc -i -s -O -DOLD_PREPROCESSOR -Dunix -DHOST=1 -DTARGET=0 -DMACHINE=\"i8088\" -DSYSTEM=\"minix\" -DCOMPILER=\"__STD_CC__\" -o cpp.mx cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"

  sc=./sc.mx
  as=./as.mx
  ld=./ld.mx
  h=0r  # Host system is Minix i86.
fi

if test "$1" = acka0 || test "$1" = acka3; then  # Minix >=1.7.0 i86 ACK ANSI C compiler (1.202 on Minix 2.0.4).  !! Autodetect acka.
  if test "$1" = acka0; then cflags="-DSMALLMEM -DINT32T=long -DINTPTRT=int -DALIGNBYTES=4"  # For Minix i86.
  else                       cflags="-DINT32T=int -DINTPTRT=int -DALIGNBYTES=4"  # For Minix i386.
  fi
  rm -f sc.mx
  # There is a bug in the cfile(...) function in /usr/src/commands/i86/cc.c
  # (both Minix 1.7.0 and 2.0.4): if the source file name starts with `',
  # the `cc' tool skips the invocation of the `irrel' tool, which causes the
  # invocation of the `cem' tool to run out of memory. Workaround we use
  # `cd sc' etc. and pass e.g. `assign.c' instead of `sc/assign.c` to
  # `cc'.
  #
  # The ACK ANSI C compiler in Minix 1.7.0 needs the `-m' flag to run
  # `irrel' on the output of the C preprocessor. Without `irrel', `ncem'
  # would run out of memory because of the too many unused declaration. The
  # ACK ANSI C compiler in Minix 2.0.4 runs `irrel' even without the `-m'
  # flag. Both use the ACK ANSI C compiler 1.202, but with a different
  # frontend (/usr/src/commands/i86/cc.c).
  #
  # /usr/lib/ncpp -D_EM_WSIZE=2 -D_EM_PSIZE=2 -D_EM_SSIZE=2 -D_EM_LSIZE=4 -D_EM_FSIZE=4 -D_EM_DSIZE=8 -D__ACK__ -D__minix -D__i86 hello.c >/tmp/cc230000.i
  (cd sc && cc -i -s -O -m $cflags -DACKFIX -o ../sc.mx mxmalloc.c bcc-cc1.c assign.c codefrag.c debug.c declare.c express.c exptree.c floatop.c function.c gencode.c genloads.c glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c softop.c state.c table.c type.c) || exit "$?"
  (cd as && cc -i -s -O -m $cflags -DMINIX_SYNTAX -DMINIXHEAP -o ../as.mx as.c assemble.c error.c express.c genbin.c genlist.c genobj.c gensym.c heap.c keywords.c macro.c mops.c pops.c readsrc.c scan.c table.c typeconv.c) || exit "$?"
  (cd ld && cc -i -s -O -m $cflags -DMINIXHEAP -DACKFIX -o ../ld.mx dumps.c heap.c io.c ld.c readobj.c table.c typeconv.c writebin.c) || exit "$?"
  (cd cr && cc -i -s -O -m $cflags -o ../cr.mx cr.c) || exit "$?"
  # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
  # -wo is specified to omit the ACK ANSI C compiler 1.202 warnings: ... old-fashioned function declaration; ... old-fashioned function definition
  (cd cpp && cc -i -s -O -m -DOLD_PREPROCESSOR -Dunix -DHOST=1 -DTARGET=0 -DMACHINE=\"i8088\" -DSYSTEM=\"minix\" -DCOMPILER=\"__STD_CC__\" -o ../cpp.mx cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c) || exit "$?"

  h=0r  # Host system is Minix i86.
  sc=./sc.mx
  as=./as.mx
  ld=./ld.mx
  cr=  # Will be set later.
  if test "$1" = acka3; then
    h=3r  # Host system is Minix i386.
    chmem =150000 sc.mx || exit "$?"  # C compiler backend.
    chmem =192480 as.mx || exit "$?"  # Assembler.
    chmem =150000 ld.mx || exit "$?"  # Linker.
    chmem  =40000 cr.mx || exit "$?"  # Library builder.
  fi
fi

if test "$1" = bcc0; then  # BCC (v0 or v3) on Minix i86, targeting Minix i86: sc, as and ld from bccbin16.tar.Z by Bruce Evans.
  bcc=bcc
  rm -f sc.mx as.mx ld.mx cr.mx cpp.mx
  # Try to find the linker filename (typically fld=/local/bin/ld) and the host Minix kernel architecture (f03=-0 for i86 or f03=-3 for i386).
  # Doing it in a subshell to avoid `argument list too long' errors later. This may be a bug in Minix 1.5.10 /bin/sh.
  ( echo "int main() { return 0; }" >t.c || exit "$?"
    fld=; f03=; rm -f t.out
    "$bcc" -v -o t t.c 2>&1 | (while read ald ao at ai a03 arest; do
      if test "$ao $at" = "-o t"; then  # Example line: /local/bin/ld -o t -i -0
        fld="$ald"; f03="$a03"; test "$ai" = -i || f03="$ai"
      fi
    done; echo "f03='$f03'; fld='$fld'" >t.out)
    eval `cat t.out` || exit "$?"  # Sets f03=... and fld=... .
    test "$fld" || exit "$?"  # Linker not found.
    test x"$f03" = x-0 || test x"$f03" = x-3 || exit "$?"  # Architecture found.
    "$fld" -h 2>&1 | grep "^usage: " || exit "$?"  # Linker does not display the `usage: ...` message.
    )
  eval `cat t.out` || exit "$?"   # Sets f03=... and fld=... .
  rm -f t.out
  test "$fld" || exit "$?"
  test "$f03" = -0 || exit "$?"
  test -f "$fld" || exit "$?"
  test -x "$fld" || exit "$?"
  cp ld/globvar.h ld/globvar  # sc v0 i86 (but not i386) has a bug: it can't find #include files with basenames this long when included from ld/io.c. We work it around by using a copy with -DGLOBVARI.
  # This would create an executable sc.mx which crashes at startup time because it contains too much code: its a.out a_text header value is >0xff00:
  #   "$bcc" -i -0 -DSMALLMEM -DLIBCH -o sc.mx sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
  # We work it around by using a custom, smaller libc (lscs0.s and mxmalloc.c) instead of the default libc.a.
  sco="lscs0i.o mxmalloc.o bcc-cc1.o assign.o codefrag.o debug.o declare.o express.o exptree.o floatop.o function.o gencode.o genloads.o glogcode.o hardop.o input.o label.o loadexp.o longop.o output.o preproc.o preserve.o scan.o softop.o state.o table.o type.o"
  rm -f $sco
  sed "s/^include.*//" <lscs0.s >lscs0i.s || exit "$?"  # as v0 abort()s on the include pseudo-op, so we manually process it with sed+cat.
  cat libt0.si >>lscs0i.s || exit "$?"

  cflags="-DSMALLMEM -DINT32T=long -DINTPTRT=int -DALIGNBYTES=4"
  "$bcc" -i -0 $cflags -DOPEN00 -DSBRK -DLIBCH -c lscs0i.s sc/mxmalloc.c sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
  "$fld" -0 -i -o sc.mx $sco || exit "$?"
  rm -f $sco
  sco=
  # In BCC driver v0, `"$bcc" -i' (separate I&D) and `"$bcc"' default both run `ld -i' separate I&D.
  "$bcc" -i -0 $cflags -DMINIX_SYNTAX -DMINIXHEAP -DBRKSIZE -DLIBCH -o as.mx as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
  "$bcc" -i -0 $cflags -DMINIXHEAP -DBRKSIZE -DGLOBVARI -DLIBCH -DLIBCHMINIX -DBCCALIGNFIX -o ld.mx ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
  "$bcc" -i -0 $cflags -DLIBCH -o cr.mx cr/cr.c || exit "$?"
  # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
  "$bcc" -i -0 -DOLD_PREPROCESSOR -DNOSTDLIBH -Dunix -DHOST=1 -DTARGET=0 -DMACHINE=\"i8088\" -DSYSTEM=\"minix\" -DCOMPILER=\"__STD_CC__\" -o cpp.mx cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"

  h=0r  # Host system is Minix i86.
  sc=./sc.mx
  as=./as.mx
  ld=./ld.mx
  cr=  # Will be set later.
fi

if test "$1" = bcc3; then  # BCC (v0 or v3) on Minix i386, targeting Minix i386: sc, as and ld from bccbin16.tar.Z by Bruce Evans.
  # !! Add support for compiling on Minix-386vm and Minix-vmd as h=5.
  # !! Add support for Minix-386vm and Minix-vmd as targets of ld. Is it just `ld -z', or some more? Adjust behavior of `ld -z'.
  bcc=bcc
  rm -f sc.mx as.mx ld.mx cr.mx cpp.mx
  cflags="-DINT32T=int -DINTPTRT=int -DALIGNBYTES=4"
  "$bcc" -i $cflags -DLIBCH -o sc.mx sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
  # In BCC driver v0, `"$bcc" -i' (separate I&D) and `"$bcc"' default both run `ld -i' separate I&D.
  "$bcc" -i $cflags -DMINIX_SYNTAX -DMINIXHEAP -DBRKSIZE -DLIBCH -o as.mx as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
  "$bcc" -i $cflags -DMINIXHEAP -DBRKSIZE -DLIBCH -DLIBCHMINIX -DBCCALIGNFIX -o ld.mx ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
  "$bcc" -i $cflags -DLIBCH -o cr.mx cr/cr.c || exit "$?"
  chmem =150000 sc.mx || exit "$?"  # C compiler backend.
  chmem =192480 as.mx || exit "$?"  # Assembler.
  chmem =150000 ld.mx || exit "$?"  # Linker.
  chmem =40000 cr.mx || exit "$?"  # Library builder.

  # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
  "$bcc" -i -DOLD_PREPROCESSOR -DNOSTDLIBH -Dunix -DHOST=1 -DTARGET=0 -DMACHINE=\"i8088\" -DSYSTEM=\"minix\" -DCOMPILER=\"__STD_CC__\" -o cpp.mx cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"
  chmem =150000 cpp.mx || exit "$?"  # C preprocessor.

  h=3r  # Host system is Minix i386.
  sc=./sc.mx
  as=./as.mx
  ld=./ld.mx
  cr=  # Will be set later.
fi

test "$h" || exit "$?"  # Unknown compilation method.

# --- Rebuild the tools if supported by the host system.

if test "$h" = 0r; then  # Rebuild the host tools (sc, as, ld etc.) using "$sc", "$as" and "$ld" etc. for Minix i86.
  sh toolsb.sh 0 "$sc" "$as" "$ld" "" "$tcmp" "$tdiff" "" || exit "$?"
  mv sc.tool sc.host || exit "$?"
  mv as.tool as.host || exit "$?"
  mv ld.tool ld.host || exit "$?"
  mv cr.tool cr.host || exit "$?"
  h=0
  sc=./sc.host
  as=./as.host
  ld=./ld.host
  cr=./cr.host
fi

if test "$h" = 3r; then  # Rebuild the host tools (sc, as, ld etc.) using "$sc", "$as" and "$ld" etc. for Minix i386.
  sh toolsb.sh 3 "$sc" "$as" "$ld" "" "$tcmp" "$tdiff" "" || exit "$?"
  mv sc.tool sc.host || exit "$?"
  mv as.tool as.host || exit "$?"
  mv ld.tool ld.host || exit "$?"
  mv cr.tool cr.host || exit "$?"
  h=3
  sc=./sc.host
  as=./as.host
  ld=./ld.host
  cr=./cr.host
fi

# Now we have our final "$sc", "$as", "$ld" tools, using which we build the target tools and the libc.
test -f "$sc" || exit "$?"
test -f "$as" || exit "$?"
test -f "$ld" || exit "$?"
test -f "$cr" || exit "$?"
test -x "$sc" || exit "$?"
test -x "$as" || exit "$?"
test -x "$ld" || exit "$?"
test -x "$cr" || exit "$?"

# --- Build the target tools for each system.
#
# We can do this before building the libc, because these tools don't use the libc.

rm -f sc.tool as.tool ld.tool cr.tool
sh toolsb.sh 0 "$sc" "$as" "$ld" "" "$tcmp" "$tdiff" "" || exit "$?"
mv sc.tool 0/sc || exit "$?"
mv as.tool 0/as || exit "$?"
mv ld.tool 0/ld || exit "$?"
mv cr.tool 0/cr || exit "$?"

rm -f sc.tool as.tool ld.tool cr.tool
sh toolsb.sh 3 "$sc" "$as" "$ld" "" "$tcmp" "$tdiff" "" || exit "$?"
mv sc.tool 3/sc || exit "$?"
mv as.tool 3/as || exit "$?"
mv ld.tool 3/ld || exit "$?"
mv cr.tool 3/cr || exit "$?"

# --- Build the target libc object files for each system.
#
# We don't need the target tools for this, because we use the host tools ("$sc", "$as", "$ld").

# Relevant flags of the assembler ($as):
# -0  start with 16-bit code segment; default: native
# -3  start with 32-bit code segment; default: native
# -a  enable partial source compatibility with Minix 1.5.10 asld
# -g  only put global symbols in object file  # !! Start using it.
# -n name.s  module name (without extension) to put to .o file
# -o name.o  produce object file
# -u  take undefined symbols as imported-with-unspecified segment
# -w  don't print warnings

idirflag=-Iinclude  # !! Make a copy, add patches (s.sh), use it as bccinclude: idirflag=include

ANSIB='abort abs assert atoi atol bsearch ctime ctype errno exit fclose fflush fgetc fgets fopen fprintf fputc fputs fread freopen fseek ftell fwrite getenv gets malloc perror puts qsort rand scanf setbuf signal sprintf strerror strtol strtoul system time tmpnam ungetc vsprintf'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/ansi . Patched by Bruce Evans.
IBMB='peekpoke portio'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/ibm . Patched by Bruce Evans.
OTHERB='amoeba bcmp bcopy brk bzero call chroot cleanup crypt curses doprintf ffs getdents getopt getpass gtty index ioctl itoa lock lrand lsearch memccpy message mknod mktemp mount nlist popen printdat printk prints ptrace putenv regexp regsub rindex seekdir stb stderr stime stty swab sync syslib telldir termcap umount uniqport vectab'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/other . Patched by Bruce Evans.
POSIXB='_exit access alarm chdir chmod chown close closedir creat ctermid cuserid dup dup2 exec execlp fcntl fdopen fork fpathconf fstat getcwd getegid geteuid getgid getgrent getlogin getpid getppid getpwent getuid isatty kill link lseek mkdir mkfifo open opendir pathconf pause pipe read readdir rename rewinddir rmdir setgid setuid sleep stat sysconf times ttyname umask unlink utime wait write'  # C source files (*.c) from Minix 1.5.10 /usr/src/lib/posix . Patched by Bruce Evans.
ASB='catchsig crtso head idiv idivu imod imodu imul isl isr isru sendrec setjmp'  # Architecture-specific assembly source files (*.s).

for a03 in 0 3; do
  if test "$a03" = 0; then  # i86.
    aflag=-a  # asld compatibility.
    SB='crtso head'  # Minix startup routines, assembly source files (*.s) by Bruce Evans.
    FB='dummy'  # BCC compiler non-support for floating point arithmetic, assembly source files (*.s) by Bruce Evans.
    IB='idiv idivu imod imodu imul isl isr isru'  # BCC compiler support for integer arithmetic, assembly source files (*.s) by Bruce Evans.
    JB='inport inportb outport outportb peekb peekw pokeb pokew'  # miscellaneous obsolete i86 junk, assembly source files (*.s) by Bruce Evans. !! Are these files needed? Probably not, remove them.
    LBB='laddb landb lcmpb lcomb ldecb ldivb ldivub leorb lincb lmodb lmodub lmulb lnegb lorb lslb lsrb lsrub lsubb ltstb'  # BCC compiler support for long arithmetic on big-endian (words-swapped) longs, assembly source files (*.s) by Bruce Evans.
    LLB='laddl landl lcmpl lcoml ldecl ldivl ldivul leorl lincl lmodl lmodul lmull lnegl lorl lsll lsrl lsrul lsubl ltstl'  # BCC compiler support for long arithmetic on little-endian (normal) longs, assembly source files (*.s) by Bruce Evans.
    GB='ldivmod'  # BCC compiler support for integer arithmetic (common long arithmetic), assembly source files (*.s) by Bruce Evans.
    MB='brksize catchsig sendrec setjmp'  # Minix library routines, assembly source files (*.s) by Bruce Evans.
    STRPASMB='memchr memcmp memmove memset strcat strchr strcmp strcoll strcpy strcspn strlen strncat strncmp strncpy strpbrk strrchr strspn strstr strtok strxfrm'  # Minix library string routines, assembly sourcefiles (*.s) created manually from to-be-preprecessed assembly source files (*.x) from Minix 1.5.10 /usr/src/lib/string . Patched by Bruce Evans.
    STRCB=''  # No additional string routines, C source files (*.c) by Bruce Evans.
    STRC2B=''  # No additional string routines, C source files (*.c) from Minix 1.5.10 /usr/src/lib/string .
  else  # i386.
    aflag=  # No asld compatibility.
    SB='crtso head'  # Minix startup routines, assembly source files (*.s) by Bruce Evans.
    FB='dummy'  # BCC compiler non-support for floating point arithmetic, assembly source files (*.s) by Bruce Evans.
    IB='idiv idivu imod imodu imul isl isr isru'  # BCC compiler support for integer arithmetic, assembly source files (*.s) by Bruce Evans.
    JB=''
    LBB=''
    LLB=''
    GB='divsi3'  # GCC compiler support for integer arithmetic, assembly source files (*.s) by Bruce Evans.
    MB='brksize catchsig sendrec setjmp'  # Minix library routines, assembly source files (*.s) by Bruce Evans.
    STRPASMB=''  # No additional Minix library string routines. These functions are implemented in STRCB and STRC2B instead.
    STRCB='memcmp memcpy memset strcmp strcpy strcspn strlen strncat strncmp strncpy strpbrk'  # Minix library string routines, C source files (*.c) with i386 inline assembly by Bruce Evans.
    STRC2B='memchr memmove strcat strchr strcoll strrchr strspn strstr strtok strxfrm'  # Minix library string routines, C source files (*.c) without assembly from Minix 1.5.10 /usr/src/lib/string . Patched by Bruce Evans.
  fi

  test -d "$a03" || mkdir "$a03" || exit "$?"
  for b in $ASB; do cp libc/"$b"_"$a03".s libc/"$b".s || exit "$?"; done

  # ($bccstringdir) $STRCB
  # To force the 8086 version (don't - the Minix asm one is better), use: make CC='bcc -0'
  # To force the 80386 version, use: make CC='bcc -3'
  # For a portable version, use: make XCFLAGS=-DC_CODE

  # ($asmstringdir) These are -0 (i86) only.
  for b in $STRPASMB; do
    "$as" -"$a03" -w $aflag -o "$a03"/"$b".o libc/"$b".s || exit "$?"
    "$cmp" "$a03"/"$b".o "$a03"g/"$b".n || exit "$?"  # Compare object file (*.o) to golden file (*.n).
  done

  # ($bccsupportdir)
  for b in $SB $FB $IB $JB $LBB $LLB $GB $MB; do  # Assembly source files (*.s) by Bruce Evans.
    # "$bcc" -v -"$a03" -c -o "$a03"/"$b".o "$b".s || exit "$?"
    "$as" -"$a03" -w $aflag -o "$a03"/"$b".o libc/"$b".s || exit "$?"
    "$cmp" "$a03"/"$b".o "$a03"g/"$b".n || exit "$?"  # Compare object file (*.o) to golden file (*.n).
  done

  for b in $ANSIB $IBMB $OTHERB $POSIXB $STRCB $STRC2B; do
    # "$bcc" -v -"$a03" -c -O -D_MINIX -D_POSIX_SOURCE -o "$a03"/"$b".o "$b".c
    "$sc" -"$a03" $idirflag -D_MINIX -D_POSIX_SOURCE -o "$a03"/"$b".s libc/"$b".c || exit "$?"
    "$as" -"$a03" -u -w -o "$a03"/"$b".o "$a03"/"$b".s || exit "$?"
    "$cmp" "$a03"/"$b".o "$a03"g/"$b".n || exit "$?"  # Compare object file (*.o) to the relavant v3 golden file (*.m).
    rm -f "$a03"/"$b".s || exit "$?"
  done

  # ($earlstdiodir) Not compiling estdio.
  # ($localdir) No directory /usr/src/lib/local .

  for b in $ASB; do rm -f libc/"$b".s || exit "$?"; done
done

# --- Build the target libc library archive (*.a) files for each system.
#
# We don't need the target tools for this, because we use the host tools ("$cr").

cru="$cr"; case "$cr" in /*) ;; */*) cru="../$cr" ;; esac  # "$cru" is "$cr", but it works from one directory lower.
# !! Increment typestamp because of changes in isatty.c etc.
# !! Redo both in alphabetic order.
(cd 0 && "$cru" -o libc.a -t @644973285 _exit.o abort.o abs.o access.o alarm.o amoeba.o assert.o atoi.o atol.o bcmp.o bcopy.o brk.o brksize.o bsearch.o bzero.o call.o catchsig.o chdir.o chmod.o chown.o chroot.o cleanup.o close.o closedir.o creat.o crypt.o ctermid.o ctime.o ctype.o curses.o cuserid.o doprintf.o dummy.o dup.o dup2.o errno.o exec.o execlp.o exit.o fclose.o fcntl.o fdopen.o fflush.o ffs.o fgetc.o fgets.o fopen.o fork.o fpathconf.o fprintf.o fputc.o fputs.o fread.o freopen.o fseek.o fstat.o ftell.o fwrite.o getcwd.o getdents.o getegid.o getenv.o geteuid.o getgid.o getgrent.o getlogin.o getopt.o getpass.o getpid.o getppid.o getpwent.o gets.o getuid.o gtty.o idiv.o idivu.o imod.o imodu.o imul.o index.o ioctl.o isatty.o isl.o isr.o isru.o itoa.o kill.o laddl.o landl.o lcmpl.o lcoml.o ldecl.o ldivl.o ldivmod.o ldivul.o leorl.o lincl.o link.o lmodl.o lmodul.o lmull.o lnegl.o lock.o lorl.o lrand.o lsearch.o lseek.o lsll.o lsrl.o lsrul.o lsubl.o ltstl.o malloc.o memccpy.o memchr.o memcmp.o memmove.o memset.o message.o mkdir.o mkfifo.o mknod.o mktemp.o mount.o nlist.o open.o opendir.o pathconf.o pause.o peekpoke.o perror.o pipe.o popen.o portio.o printdat.o printk.o prints.o ptrace.o putenv.o puts.o qsort.o rand.o read.o readdir.o regexp.o regsub.o rename.o rewinddir.o rindex.o rmdir.o scanf.o seekdir.o sendrec.o setbuf.o setgid.o setjmp.o setuid.o signal.o sleep.o sprintf.o stat.o stb.o stderr.o stime.o strcat.o strchr.o strcmp.o strcoll.o strcpy.o strcspn.o strerror.o strlen.o strncat.o strncmp.o strncpy.o strpbrk.o strrchr.o strspn.o strstr.o strtok.o strtol.o strtoul.o strxfrm.o stty.o swab.o sync.o sysconf.o syslib.o system.o telldir.o termcap.o time.o times.o tmpnam.o ttyname.o umask.o umount.o ungetc.o uniqport.o unlink.o utime.o vectab.o vsprintf.o wait.o write.o) || exit "$?"
"$cmp" 0/libc.a 0g/libc.d || exit "$?"
#cmp -l 0/libc.w 0/libc.a ||:  # Mismatch in the even padding bytes.
(cd 3 && "$cru" -o libc.a -t @645451200 abort.o abs.o assert.o atoi.o atol.o bsearch.o ctime.o ctype.o errno.o exit.o fclose.o fflush.o fgetc.o fgets.o fopen.o fprintf.o fputc.o fputs.o fread.o freopen.o fseek.o ftell.o fwrite.o getenv.o gets.o malloc.o memchr.o memmove.o perror.o puts.o qsort.o rand.o scanf.o setbuf.o signal.o sprintf.o strcat.o strchr.o strcoll.o strerror.o strrchr.o strspn.o strstr.o strtok.o strtol.o strtoul.o strxfrm.o system.o time.o tmpnam.o ungetc.o vsprintf.o peekpoke.o portio.o amoeba.o bcmp.o bcopy.o brk.o bzero.o call.o chroot.o cleanup.o crypt.o curses.o doprintf.o ffs.o getdents.o getopt.o getpass.o gtty.o index.o ioctl.o itoa.o lock.o lrand.o lsearch.o memccpy.o message.o mknod.o mktemp.o mount.o nlist.o popen.o printdat.o printk.o prints.o ptrace.o putenv.o regexp.o regsub.o rindex.o seekdir.o stb.o stderr.o stime.o stty.o swab.o sync.o syslib.o telldir.o termcap.o umount.o uniqport.o vectab.o _exit.o access.o alarm.o chdir.o chmod.o chown.o close.o closedir.o creat.o ctermid.o cuserid.o dup.o dup2.o exec.o execlp.o fcntl.o fdopen.o fork.o fpathconf.o fstat.o getcwd.o getegid.o geteuid.o getgid.o getgrent.o getlogin.o getpid.o getppid.o getpwent.o getuid.o isatty.o kill.o link.o lseek.o mkdir.o mkfifo.o open.o opendir.o pathconf.o pause.o pipe.o read.o readdir.o rename.o rewinddir.o rmdir.o setgid.o setuid.o sleep.o stat.o sysconf.o times.o ttyname.o umask.o unlink.o utime.o wait.o write.o memcmp.o memcpy.o memset.o strcmp.o strcpy.o strcspn.o strlen.o strncat.o strncmp.o strncpy.o strpbrk.o dummy.o divsi3.o idiv.o idivu.o imod.o imodu.o imul.o isl.o isr.o isru.o brksize.o catchsig.o sendrec.o setjmp.o) || exit "$?"
"$cmp" 3/libc.a 3g/libc.d || exit "$?"
#cmp -l 3/libc.w 3/libc.a ||:  # Mismatch in $uid, $gid, $mtime and in the even padding bytes.

# --- Build the extra tools for each system.
#
# We don't need the target tools for this, because we use the host tools
# ("$sc", "$as", "$ld"), but we need the target libc ("$a03"/crtso.o and
# "$a03"/libc.a).

"$as" -0 -a -w -o 0/lcpps.o cpp/lcpps0.s || exit "$?"  # It will be unused so far.
"$cmp" 0/lcpps.o 0g/lcpps.n || exit "$?"
for a03 in 0 3; do
  for b in cpp1 cpp2 cpp3 cpp4 cpp5 cpp6; do
    "$sc" -"$a03" "$idirflag" -DOLD_PREPROCESSOR -Dunix -DHOST=1 -DTARGET=0 -DMACHINE=\"i8088\" -DSYSTEM=\"minix\" -DCOMPILER=\"__STD_CC__\" -o "$a03"/cpp"$b".s cpp/"$b".c || exit "$?"
    "$diff" "$a03"/cpp"$b".s "$a03"g/cpp"$b".r || exit "$?"
    "$as" -"$a03" -u -w -o "$a03"/cpp"$b".o "$a03"/cpp"$b".s || exit "$?"
    "$cmp"  "$a03"/cpp"$b".o "$a03"g/cpp"$b".n || exit "$?"
  done
  rm -f cpp
  if test "$a03" = 3; then chmemval=150000; else chmemval=0; fi  # chmem =150000 3/cpp || exit "$?"  # C preprocessor.
  "$ld" -"$a03" -i -h "$chmemval" -o "$a03"/cpp "$a03"/crtso.o "$a03"/cppcpp1.o "$a03"/cppcpp2.o "$a03"/cppcpp3.o "$a03"/cppcpp4.o "$a03"/cppcpp5.o "$a03"/cppcpp6.o "$a03"/libc.a || exit "$?"
  ls -l "$a03"/cpp >&2 || exit "$?"
  "$cmp" "$a03"/cpp "$a03"g/cpp.d || exit "$?"

  # !! Add "$cmp" for cc.
  for b in cc; do
    "$sc" -"$a03" -Iinclude   -o "$a03"/cc"$b".s cc/cc.c || exit "$?"
    "$as" -"$a03" -u -w       -o "$a03"/cc"$b".o "$a03"/cc"$b".s || exit "$?"
  done
  "$ld" -"$a03" -i -h 10000   -o "$a03"/cc "$a03"/crtso.o "$a03"/cc"$b".o "$a03"/libc.a || exit "$?"  # No BSS. -h 10000 leaves >=9 KiB for the command-line arguments and environment.
done

rm -f bin/cc
case "$h" in [0-9]) cp "$h"/cc bin/bbcc ;; esac  # !! Preserve mtime when copying with `cp -p'. The Minix 1.5.10 `cp' tool doesn't support `-p'. Build our own cp if needed, or use tar?

# --- Remove temporary ?/*.[os] files.

if test "$cmp","$tcmp" = true,true || test "$cmp","$tcmp" = cmp,cmp ; then
  for a03 in 0 3; do
    mv "$a03"/crtso.o "$a03"/crtso.ok || exit "$?"  # Rename it so it will be kept.
    (rm -f "$a03"/[a-m]*.o) || exit "$?"  # Avoid the `argument list too long' error.
    (rm -f "$a03"/*.o) || exit "$?"
    mv "$a03"/crtso.ok "$a03"/crtso.o || exit "$?"
    (rm -f "$a03"/*.s) || exit "$?"
  done
fi

# --- Done.

: "$0" OK.
