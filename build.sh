#!/bin/sh
#
# build.sh: build the BCC compiler tools, libc and extr tools
# by pts@fazekas.hu at Thu Jan  8 15:51:45 CET 2026
#
# !! make it work with `minicc --gcc=4.2' (missing memcpy), `minicc --pcc' (missing prototypes for cpp; code generation difference in sc), `minicc --tcc' (missing memcpy) and `minicc --utcc' (ld.cross segfaults)
# !! simplify as/typeconv.c and ld/typeconv.c
# !! Does sc v3 support function returning struct (i.e. no error: function returning structure is illegal) ? Yes, with memcpy! Modify include/stdlib.h etc.
# !! remove default -0 and -3 from cross-compiler sc, as, ld, cc
# !! add driver flag -E to invoke CPP
# !! for some systems, set S_ALIGNMENT=1 in ld/config.h
# !! (after porting to ack) Remove the `t' symbols from `nm as.mx' etc. added by the assembler; use symbols starting with . instead?
# !! Make the `bcc -i` and `ld -i` (separate I&D) default independent of the BCC driver version.
# !! Is the `j' jump output of sc v3 compatible with as0? (as0 expects jmp as short jump.)
# !! Add the remaining patches to cpp.
# !! fix suboptimal i386 code generation in ld for `v += (unsigned long) (unsigned char) c;'; this already uses movzx: `v += (unsigned char) c;'
# !! Add support for -nostdlib in the bbcc driver.
# !! Move all libc variables from .data to .bss (with .comm).
# !! everywhere (sc, as, ld, cpp), there are about 26 instances remaining: grep '[(] *unsigned *char *[)]' {??,cpp}/*.[ch]
#    ifdef ACKFIX  /* Workaround for the bug in Minix 1.5.10 i86 ACK 3.1 C compiler, which sign-extends the (unsigned char) cast. */
#    #  define UCHARCAST(c) (unsigned char) ((unsigned) (c) & 0xff)
#    #else
#    #  define UCHARCAST(c) ((unsigned char) (c))
#    #endif
#

test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).
# test "$0" = "${0%/*}" || cd "${0%/*}" || exit "$?"

if test "$1" = --overwrite-golden; then
  cmp=cp; diff=cp; tcmp="$cmp"; tdiff="$diff"; shift  # Overwrite golden files (*r, *.n, *.d). This is dangerous.
  exit 5  # !! Dangerous.
elif test "$1" = --nocmp; then
  cmp=:; diff=:; tcmp="$cmp"; tdiff="$diff"; shift  # Ignore golden files.
elif test "$1" = --notoolscmp; then
  cmp=cmp; diff=diff; tcmp=:; tdiff=:; shift  # Ignore golden files for tools, compare generated files to non-tools golden files.
elif test "$1" = --toolsecho; then  # This works with Linux /bin/sh, but not with Minix /bin/sh.
  cmp=:; diff=:; tcmp=tcmp; tdiff=tdiff; shift  # Compare generated files to non-tools golden files.
  : >tools.diff || exit "$?"
elif test "$1" = --toolscp; then  # This works with Linux /bin/sh, but not with Minix /bin/sh.
  cmp=:; diff=:; tcmp=cp; tdiff=cp; shift  # Compare generated files to non-tools golden files. Overwrite golden files (*.r, *.n, *.d) for tools, which is dangerous.
  : >tools.diff || exit "$?"
elif test "$1" = --cmp; then
  cmp=cmp; diff=diff; tcmp="$cmp"; tdiff="$diff"; shift  # Compare generated files to golden files.
else  # Default: if directories 0g and 3g exist, then use cmd and diff, otherwise ignore comparisons.
  if test -d 0g && test -d 3g; then cmp=cmp; diff=diff
  else cmp=:; diff=:  # `cmp=true' would also work.
  fi
  tcmp="$cmp"; tdiff="$diff"
fi
test "$1" || set auto $@  # Default: autodetect the compiler.

set -x  # Echo all commands run.
h=  # Host system unnown so far. We also use `test "$h"' below to check if we've recognized "$1".

for a03 in 0 3 bin; do
  if test -d "$a03"; then :; else
    mkdir "$a03" || exit "$?"
    test -d "$a03" || exit "$?"
  fi
done

# --- Build the host tools or cross-compilation tools: "$sc", "$as", "$ld", "$cr".

if test "$1" = auto; then  # Detect if our best bet is cross-compilation with GCC.
  rm -f sysftype
  gcc -s -O2 -Wall -W -Dfoo -o sysftype sysftype.c && test "`./sysftype ./sysftype`" = "?" && shift && set gcc $@ -s -O2
fi

# Cross-compilation with GCC (gcc) or Clang (clang) (e.g. on Linux, FreeBSD, macOS), OpenWatcom v2 (owcc) on Linux, minilibc686 minicc on Linux, or a generic Unix C compiler (cc) on Unix.
if test "$1" = gcc || test "$1" = clang || test "$1" = owcc || test "$1" = minicc || test "$1" = bbcc || test "$1" = cross || test "$1" = zig; then
  # GCC is known to work with GCC 4.3--4.9 and GCC 7.5.0.
  # Clang is known to work with Clang 6.0.0.
  # Example invocation for OpenWatcom v2 on Linux: ./build.sh owcc
  # Example invocation with zig cc Clang on Linux: ./build.sh zig cc
  # Example invocation with bbcc (itself) on Minix i386 or cross-compilation on Linux: ./build.sh bbcc
  # Example strict mode invocations:
  # * GCC on modern Unix-like systems: .  /build.sh gcc   -s -O2 -DDEBUG_SIZE_NOPAD -W -Wall -Werror -Wstrict-prototypes -Wno-maybe-uninitialized -ansi -pedantic
  # * Clang on modern Unix-like systems: ./build.sh clang -s -O2 -DDEBUG_SIZE_NOPAD -W -Wall -Werror -Wstrict-prototypes -Wno-maybe-uninitialized -Wno-deprecated-non-prototype -Wno-unknown-warning-option -Wno-unused-command-line-argument -ansi -pedantic
  # * OpenWatcom v2 on Linux:            ./build.sh owcc  -s -O2 -DDEBUG_SIZE_NOPAD -W -Wall -Werror
  # * minilibc386 and OpenWatcom v2:     ./build.sh minicc -DDEBUG_SIZE_NOPAD -Werror
  # * minilibc386 and GCC 4.8:           ./build.sh minicc --gcc=4.8 -DDEBUG_SIZE_NOPAD -Werror
  # * zig cc targeting Linux i386:       ./build.sh zig cc -target i386-linux-musl      -s -O2 -W -Wall -ansi -pedantic  # Little-endian.
  # * zig cc targeting Linux amd64:      ./build.sh zig cc -target x86_64-linux-musl    -s -O2 -W -Wall -ansi -pedantic  # Little-endian.
  # * zig cc targeting Linux PowerPC:    ./build.sh zig cc -target powerpc-linux-musl   -s -O2 -W -Wall -ansi -pedantic  # Big-endian.
  # * zig cc targeting Linux PowerPC64:  ./build.sh zig cc -target powerpc64-linux-musl -s -O2 -W -Wall -ansi -pedantic  # Big-endian.
  # * zig cc targeting Linux ARM:        ./build.sh zig cc -target arm-linux-musleabi   -s -O2 -W -Wall -ansi -pedantic  # Little-endian.
  # * zig cc targeting Linux ARM64:      ./build.sh zig cc -target aarch64-linux-musl   -s -O2 -W -Wall -ansi -pedantic  # Little-endian.
  # minicc is from http://github.com/pts/minilibc686
  # Assumptions about the host system:
  # * .text can be 75 KiB. This doesn't hold for ELKS and Minix i86.
  # * There is 140 KiB of virtual memory for each process (total of: .text, .rodata, .data, .bss, stack). This doesn't hold for ELKS and Minix i86.
  # * malloc(...) can allocate 192 KiB on top of that. This doesn't hold for ELKS and Minix i86.
  # * There is no need to declare the maximum memory use of a program (including the use of malloc(...)) at compile time. This doesn't hold for ELKS, Minix i86 and Minix i386. For these system, chmem (or `ld -h ...') has to be used. !! Autodetect this.
  # Doing it in a subshell to avoid `argument list too long' errors later. This may be a bug in Minix 1.5.10 /bin/sh.
  rm -f h.out
  ( rm -f sysdet
    cc2=
    # -fno-lto is needed by `zig cc -target powerpc64-linux-musl -s -O2 -W -Wall -Werror -Wno-strict-prototypes -Wno-unused-command-line-argument -fno-lto -ansi -pedantic', which would crash in a trap instruction before main() without -fno-lto.
    if test "$1" = zig && test "$2" = cc; then cc="$1"; shift; cc2="$1"; shift; cflags="-O -Wno-unknown-warning-option -Wno-deprecated-non-prototype -Wno-unused-command-line-argument -Wno-strict-prototypes -fno-lto"  # Example: ./build.sh zig cc
    elif test "$1" = clang; then cc="$1"; shift; cflags="-O -Wno-unknown-warning-option -Wno-deprecated-non-prototype -Wno-strict-prototypes"  # Example: ./build.sh clang
    elif test "$1" = minicc; then cc="$1"; shift; cflags=  # It optimizes for size better by default than with -O.
    elif test "$1" = cross; then cc=cc; shift; cflags=-O  # A generic Unix C compiler named `cc'.
    else cc="$1"; shift; cflags=-O
    fi
    test "$1" = -O0 && shift && cflags=  # Cancel the -O, in case the C compiler doesn't support it.
    # "$@" would be correct, but Minix 1.5.10 /bin/sh doesn't do word splitting on it. So we just use $@, with forced word splitting.
    "$cc" $cc2 $cflags $@ -o sysdet sysdet.c || exit "$?"
    sysdet="`./sysdet ./sysdet`"  # Typically: sysdet="-DINT32T=int -DINTPTRT=int -DALIGNBYTES=4 -DNOPORTALIGN=0"  # !! Add -DMINALIGNBYTES=1
    test "$?" = 0 || exit 2
    rm -f sysdet
    case "$sysdet" in *-DBAD* | "") exit 3 ;; *-DINTPTRT=*) ;; *) exit 4 ;; esac
    case "$sysdet" in
     *-DOSID=3*) h=3 ;;
     *-DOSID=0*) h=0 ;;
     *) h=cross  # Host system is a cross-compiler.
    esac
    echo "$h" >h.out || exit "$?"
    test "$h" = 0 && exit 0   # Targeting Minix i86 this way won't work, because the code size (a_text) of sc would be too large (> 0xff00) with the default libc.
    # !! Autodetect the -DACKFIX etc. flags in $sysdet in case acka is used as cc.
    test -d libexec || mkdir libexec || exit "$?"
    rm -f libexec/sc libexec/as libexec/ld libexec/cr bin/bbcc libexec/cpp
    "$cc" $cc2 $cflags $sysdet $@ -o libexec/sc sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
    "$cc" $cc2 $cflags $sysdet $@ -o libexec/as as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
    "$cc" $cc2 $cflags $sysdet $@ -o libexec/ld ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
    "$cc" $cc2 $cflags $sysdet $@ -o libexec/cr cr/cr.c || exit "$?"
    "$cc" $cc2 $cflags $sysdet $@ -o bin/bbcc cc/cc.c || exit "$?"
    "$cc" $cc2 $cflags $@ -o libexec/cpp cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"
  ) || exit "$?"
  h="`cat h.out`"
  rm -f h.out
  test "$h" || exit "$?"
  if test "$h" != 0; then
    set cross  # For subsequent `test "$1" = ...'.
    sc=libexec/sc
    as=libexec/as
    ld=libexec/ld
    cr=libexec/cr
  fi
fi
if test "$1" != cross; then
  rm -f sc.mx as.mx ld.mx cr.mx cpp.mx
  cc="$1"
  if test "$1" = bbcc || test "$1" = bcc || test "$1" = cc || test "$1" = auto; then  # Autodetect the C compiler and the Minix host system.
    test "$1" = auto && cc="bbcc bcc cc"
    h=
    for cc in $cc; do  # Find first working compiler.
      rm -f sysftype
      if "$cc" -O -o sysftype sysftype.c; then
        h="`./sysftype ./sysftype`"
        test "$h" = 0 && break
        test "$h" = 3 && break
        h=
      fi
    done
    rm -f sysftype
    test "$h" = 0 || test "$h" = 3 || exit 6  # Cross-compilation (h="??") is not supported here (see above).
    if test "$cc" = cc; then  # Running on Minix x86 host, targeting Minix x86, using `cc' as the C compiler. Detect which C compiler it actually is.
      # Now: "$1" == "cc" or "$1" == auto.
      # Doing it in a subshell to avoid `argument list too long' errors later. This may be a bug in Minix 1.5.10 /bin/sh.
      ( m=cc; rm -f t t.out  # The default is using cc as a cross-compiler.
        "$cc" -v -o t sysftype.c 2>&1 | (while read acmd arest; do  # Use `-v' to see which commands cc runs.
          case "$acmd" in
           */asld) m=acko ;;
           */ncpp | *.ansi) m=acka ;;  # In Minix 1.7.0 i386, the preprocessor is /usr/lib/em_cemcom.ansi.
           */sc | */bcc-*) m=bcc ;;  # bcc or bbcc. We don't care.
          esac
        done; echo "$m" >t.out))
      m="`cat t.out`" || exit "$?"
      rm -f t t.out
      test "$m" || exit 2
      test "$m" = cc && exit 2  # Failed to autodetect the actual C compiler.
      set "$m"  # $1 := "$m" (acko, acka or bcc). Keep "$cc" == "cc".
    elif test "$1" = auto; then
      set "$cc"  # "bbcc" or "bcc".
    fi
  elif test "$1" = acko || test "$1" = acka; then  # Autodetect the Minix host system.
    cc=cc
    rm -f sysftype
    "$cc" -O -o sysftype sysftype.c || exit "$?"
    h="`./sysftype ./sysftype`"
    rm -f sysftype
    test "$h" = 0 || test "$h" = 3 || exit 6  # Cross-compilation (h="??") is not supported here (see above).
  elif test "$1" = localbin; then  # Autodetect the Minix host system.
    sc=/local/bin/sc
    as=/local/bin/as
    ld=/local/bin/ld
    "$as" -3 -o true3.o true3.s || exit "$?"
    "$ld" -3 -i -o true3 true3.o || exit "$?"
    test -x true3 || exit "$?"
    if ./true3; then  # Minix i386.
      h=3  # Host system is Minix i386.
    else
      "$as" -0 -o true0.o true0.s || exit "$?"
      "$ld" -0 -i -o true0 true0.o || exit "$?"
      ./true0 || exit "$?"  # Neither Minix i86 nor Minix i386 detected.
      h=0  # Host system is Minix i86.
    fi
  elif test "$1" = bccbin; then  # Autodetect the Minix host system.
    if test -f bccbin32/sc && test -x bccbin32/sc && bccbin32/sc </dev/null >/dev/null; then
      sc=bccbin32/sc
      as=bccbin32/as
      ld=bccbin32/ld
      h=3
    elif test -f bccbin16/sc && test -x bccbin16/sc && bccbin16/sc </dev/null >/dev/null; then
      sc=bccbin16/sc
      as=bccbin16/as
      ld=bccbin16/ld
      h=0
    else
      : "fatal: bccbin tools not found"; exit 2
    fi
  else
    : "unknown Minix C compilation method: $1"; exit 7
  fi
fi

case "$h" in [03]) ;; cross) test "$1" = cross || exit 8 ;; *) exit 9 ;; esac  # Just a sanity check. Unknown host system.

if test "$1" = localbin || test "$1" = bccbin; then  # Use /local/bin/{sc,as,ld} v0 from BCC v0 bccbin16.tar.Z or bccbin32.tar.Z on Minix to build the tools.
  # "$sc", "$as", "$ld" and "$h" are already set correctly.
  if test "$h" = 3; then
    sh toolsb.sh 3 "$sc" "$as" "$ld" "" true true i || exit "$?"
  else
    cp ld/globvar.h ld/globvar  # sc v0 i86 (but not i386) has a bug: it can't find #include files with basenames this long when included from ld/io.c. We work it around by using a copy with -DGLOBVARI.
    sh toolsb.sh 0 "$sc" "$as" "$ld" "-DGLOBVARI" true true i || exit "$?"
  fi
  mv sc.tool sc.mx || exit "$?"
  mv as.tool as.mx || exit "$?"
  mv ld.tool ld.mx || exit "$?"
  rm -f cr.tool
fi

if test "$1" = acko; then  # Minix 1.5.10 i86 old (traditional, pre-ANSI) ACK 3.1 C compiler with either the old (1990-06-01) or the new asld (1994-03-29).
  if test "$h" = 0; then
    cflags="-DSMALLMEM -DINT32T=long -DINTPTRT=int -DALIGNBYTES=4"
    # -i for separate I&D (BCC has it by default)
    # -s for adding symbols (opposite meaning as in BCC bcc and GCC gcc)
    "$cc" -i    -O $cflags -DOPEN00 -DNOUNIONINIT -DACKFIX -DSBRK -DIDIVTOZ -DLIBCH -DMXMALLOC -c sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
    # This old asld (but not the new asld) would run out of memory if we used the Minix 1.5.10 libc (/usr/lib/crtso.c ... /usr/lib/libc.a /usr/lib/end.s).
    # We work it around by using a small, custom libc tailored for sc (lsca.s bcc-cc1. ... lsce.s).
    asld -i -o sc.mx sc/lsca.s bcc-cc1.s assign.s codefrag.s debug.s declare.s express.s exptree.s floatop.s function.s gencode.s genloads.s glogcode.s hardop.s input.s label.s loadexp.s longop.s output.s preproc.s preserve.s scan.s softop.s state.s table.s type.s sc/lsce.s || exit "$?"
    rm -f bcc-cc1.s assign.s codefrag.s debug.s declare.s express.s exptree.s floatop.s function.s gencode.s genloads.s glogcode.s hardop.s input.s label.s loadexp.s longop.s output.s preproc.s preserve.s scan.s softop.s state.s table.s type.s
    "$cc" -i -s -O $cflags -DMINIXHEAP -DBRKSIZE -o as.mx as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
    "$cc" -i -s -O $cflags -DMINIXHEAP -DBRKSIZE -DACKFIX -o ld.mx ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
    "$cc" -i -s -O $cflags -o cr.mx cr/cr.c || exit "$?"
    # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
    "$cc" -i -s -O -o cpp.mx cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"
  fi
fi

if test "$1" = acka; then  # Minix >=1.7.0 i86 or i386 ACK ANSI C compiler (1.202 on Minix 2.0.4).
  cflags="-DINT32T=int -DINTPTRT=int -DALIGNBYTES=4"  # For Minix i386.
  test "$h" = 0 && cflags="-DSMALLMEM -DINT32T=long -DINTPTRT=int -DALIGNBYTES=4"  # For Minix i86.
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
  (cd sc && "$cc" -i -s -O -m $cflags -DACKFIX -o ../sc.mx bcc-cc1.c assign.c codefrag.c debug.c declare.c express.c exptree.c floatop.c function.c gencode.c genloads.c glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c softop.c state.c table.c type.c) || exit "$?"
  (cd as && "$cc" -i -s -O -m $cflags -DMINIXHEAP -o ../as.mx as.c assemble.c error.c express.c genbin.c genlist.c genobj.c gensym.c heap.c keywords.c macro.c mops.c pops.c readsrc.c scan.c table.c typeconv.c) || exit "$?"
  (cd ld && "$cc" -i -s -O -m $cflags -DMINIXHEAP -DACKFIX -o ../ld.mx dumps.c heap.c io.c ld.c readobj.c table.c typeconv.c writebin.c) || exit "$?"
  (cd cr && "$cc" -i -s -O -m $cflags -o ../cr.mx cr.c) || exit "$?"
  # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
  # -wo is specified to omit the ACK ANSI C compiler 1.202 warnings: ... old-fashioned function declaration; ... old-fashioned function definition
  (cd cpp && "$cc" -i -s -O -m -o ../cpp.mx cpp1.c cpp2.c cpp3.c cpp4.c cpp5.c cpp6.c) || exit "$?"
  cr=  # Will be set later.
fi

if test "$1" = bcc || test "$1" = bbcc; then  # BCC (v0 or v3) or minixbcc (bbcc) on Minix x86, targeting Minix x86.
  if test "$h" = 0; then  # Host system is Minix i86.
    # Try to find the linker filename (typically fld=/local/bin/ld) and the host Minix kernel architecture (f03=-0 for i86 or f03=-3 for i386).
    # !! Simplify for bbcc: fld="bbcc ld" (and use it as $fld, without the quotes)
    # Doing it in a subshell to avoid `argument list too long' errors later. This may be a bug in Minix 1.5.10 /bin/sh.
    ( echo "int main() { return 0; }" >t.c || exit "$?"
      fld=; f03=; rm -f t.out
      "$cc" -v -o t t.c 2>&1 | (while read ald ao at ai a03 arest; do
        if test "$ao $at" = "-o t"; then  # Example line: /local/bin/ld -o t -i -0
          fld="$ald"; f03="$a03"; test "$ai" = -i || f03="$ai"
        fi
      done; echo "f03='$f03'; fld='$fld'" >t.out)
      eval `cat t.out` || exit "$?"  # Sets f03=... and fld=... .
      test "$fld" || exit "$?"  # Linker not found.
      test x"$f03" = x-0 || test x"$f03" = x-3 || exit "$?"  # Architecture found.
      "$fld" -h 2>&1 | grep "^usage: " || exit "$?"  # Linker does not display the `usage: ...` message.
      ) || exit "$?"
    eval `cat t.out` || exit "$?"   # Sets f03=... and fld=... .
    rm -f t.out
    test "$fld" || exit "$?"
    test "$f03" = -0 || exit "$?"
    test -f "$fld" || exit "$?"
    test -x "$fld" || exit "$?"
    cp ld/globvar.h ld/globvar  # sc v0 i86 (but not i386) has a bug: it can't find #include files with basenames this long when included from ld/io.c. We work it around by using a copy with -DGLOBVARI.
    # This would create an executable sc.mx which crashes at startup time because it contains too much code: its a.out a_text header value is >0xff00:
    #   "$cc" -i -0 -DSMALLMEM -DLIBCH -o sc.mx sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
    # We work it around by using a custom, smaller libc (lscs0.s and *malloc.c) instead of the default libc.a.
    sco="lscs0i.o bcc-cc1.o assign.o codefrag.o debug.o declare.o express.o exptree.o floatop.o function.o gencode.o genloads.o glogcode.o hardop.o input.o label.o loadexp.o longop.o output.o preproc.o preserve.o scan.o softop.o state.o table.o type.o"
    rm -f $sco
    sed "s/^include.*//" <sc/lscs0.s >lscs0i.s || exit "$?"  # as v0 abort()s on the include pseudo-op, so we manually process it with sed+cat.
    cat libt0.si >>lscs0i.s || exit "$?"

    cflags="-DSMALLMEM -DINT32T=long -DINTPTRT=int -DALIGNBYTES=4"
    "$cc" -i -0 $cflags -DOPEN00 -DSBRK -DMXMALLOC -DLIBCH -c lscs0i.s sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
    "$fld" -0 -i -o sc.mx $sco || exit "$?"
    rm -f $sco
    sco=
    # In BCC driver v0, `"$cc" -i' (separate I&D) and `"$cc"' default both run `ld -i' separate I&D.
    "$cc" -i -0 $cflags -DMINIXHEAP -DBRKSIZE -DLIBCH -o as.mx as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
    "$cc" -i -0 $cflags -DMINIXHEAP -DBRKSIZE -DGLOBVARI -DLIBCH -DLIBCHMINIX -o ld.mx ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
    "$cc" -i -0 $cflags -DLIBCH -o cr.mx cr/cr.c || exit "$?"
    # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
    "$cc" -i -0 -DNOSTDLIBH -o cpp.mx cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"
  elif test "$h" = 3; then  # Host system is Minix i386.
    # !! Add support for compilation on Minix-386vm and Minix-vmd as h=5.
    # !! Add support for Minix-386vm and Minix-vmd as targets of ld. Is it just `ld -z', or some more? Adjust behavior of `ld -z'.
    cflags="-DINT32T=int -DINTPTRT=int -DALIGNBYTES=4"
    "$cc" -i $cflags -DLIBCH -o sc.mx sc/bcc-cc1.c sc/assign.c sc/codefrag.c sc/debug.c sc/declare.c sc/express.c sc/exptree.c sc/floatop.c sc/function.c sc/gencode.c sc/genloads.c sc/glogcode.c sc/hardop.c sc/input.c sc/label.c sc/loadexp.c sc/longop.c sc/output.c sc/preproc.c sc/preserve.c sc/scan.c sc/softop.c sc/state.c sc/table.c sc/type.c || exit "$?"
    # In BCC driver v0, `"$cc" -i' (separate I&D) and `"$cc"' default both run `ld -i' separate I&D.
    "$cc" -i $cflags -DMINIXHEAP -DBRKSIZE -DLIBCH -o as.mx as/as.c as/assemble.c as/error.c as/express.c as/genbin.c as/genlist.c as/genobj.c as/gensym.c as/heap.c as/keywords.c as/macro.c as/mops.c as/pops.c as/readsrc.c as/scan.c as/table.c as/typeconv.c || exit "$?"
    "$cc" -i $cflags -DMINIXHEAP -DBRKSIZE -DLIBCH -DLIBCHMINIX -o ld.mx ld/dumps.c ld/heap.c ld/io.c ld/ld.c ld/readobj.c ld/table.c ld/typeconv.c ld/writebin.c || exit "$?"
    "$cc" -i $cflags -DLIBCH -o cr.mx cr/cr.c || exit "$?"
    # A working cpp is not needed for building minixbcc, we can build it later using the fully built minixbcc. !! We use it just to get rid of the warnings. Comment it out.
    "$cc" -i -DNOSTDLIBH -o cpp.mx cpp/cpp1.c cpp/cpp2.c cpp/cpp3.c cpp/cpp4.c cpp/cpp5.c cpp/cpp6.c || exit "$?"
  fi
fi

# --- Rebuild the tools if supported by the host system.

if test "$h" != cross; then  # Rebuild the host tools (sc, as, ld etc.) using "$sc", "$as" and "$ld" etc. for Minix i86 and i386.
  test -f sc.mx || exit "$?"
  if test "$h" = 3; then
    chmem =150000 sc.mx  || exit "$?"  # C compiler backend.
    chmem =192480 as.mx  || exit "$?"  # Assembler.
    chmem =150000 ld.mx  || exit "$?"  # Linker.
    chmem  =40000 cr.mx  || exit "$?"  # Library builder.
    chmem =150000 cpp.mx || exit "$?"  # C preprocessor.
  fi
  sh toolsb.sh "$h" ./sc.mx ./as.mx ./ld.mx "" "$tcmp" "$tdiff" "" || exit "$?"
  mv sc.tool sc.host || exit "$?"
  mv as.tool as.host || exit "$?"
  mv ld.tool ld.host || exit "$?"
  mv cr.tool cr.host || exit "$?"
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
# We don't need the target tools for this, because we use the host tools ("$sc", "$as").

sh cross.sh --libc "$sc" "$as" "$cr" "$cmp" || exit "$?"

# --- Build the target libc library archive (*.a) files for each system.
#
# We don't need the target tools for this, because we use the host tools ("$cr").

:  # cross.sh above has done it already.

# --- Build the extra tools for each system.
#
# We don't need the target tools for this, because we use the host tools
# ("$sc", "$as", "$ld"), but we need the target libc ("$a03"/crtso.o and
# "$a03"/libc.a).

for a03 in 0 3; do
  for b in cpp1 cpp2 cpp3 cpp4 cpp5 cpp6; do
    "$sc" -"$a03" -Iinclude -o "$a03"/cpp"$b".s cpp/"$b".c || exit "$?"
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
    "$sc" -"$a03" -Iinclude -DNOCROSS -o "$a03"/cc"$b".s cc/cc.c || exit "$?"
    "$as" -"$a03" -u -w -o "$a03"/cc"$b".o "$a03"/cc"$b".s || exit "$?"
  done
  "$ld" -"$a03" -i -h 10000   -o "$a03"/cc "$a03"/crtso.o "$a03"/cc"$b".o "$a03"/libc.a || exit "$?"  # No BSS. -h 10000 leaves >=9 KiB for the command-line arguments and environment.
done

# Don't overwrite the cross-compiler bin/bbcc.
case "$h" in [0-9]) rm -f bin/bbcc.inst; cp "$h"/cc bin/bbcc.inst ;; esac  # !! Preserve mtime when copying with `cp -p'. The Minix 1.5.10 `cp' tool doesn't support `-p'. Build our own cp if needed, or use tar?

# --- Remove temporary ?/*.[os] files.

if test "$cmp","$tcmp" = true,true || test "$cmp","$tcmp" = :,: || test "$cmp","$tcmp" = cmp,cmp ; then
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
