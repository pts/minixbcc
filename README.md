# minixbcc: fully open-source, BCC-based C compiler for Minix i86 and i386

minixbcc is a fully open-source, K&R, non-optimizing C compiler for Minix
i86 and i386, based on BCC (Bruce's C compiler, nowadays part of Dev86). It
also contains an assembler (As86), a linker, and its own libc and C include
files. It runs on Minix 1.5.10--2.0.4 i86 and i386, and it targets Minix
1.5.10 (and possibly later, including 2.0.4). It can also run as a
cross-compiler on Linux, Windows WSL, macOS, FreeBSD, OpenBSD, NetBSD,
Dragonfly BSD, ELKS, Docker and possibly other, modern Unix-like systems. It
can be compiled on Minix 1.5.10--2.0.4 i86 and i386 using the system's
default C compiler (one of the few, Minix-specific variants of ACK), or the
initial, non-open-source BCC, or itself.

The license of minixbcc is GNU GPL v2, because that's the license of BCC,
the C compiler it is based on.

The components of minixbcc (all included as source):

* the driver (bbcc, cc)
* the frontend--backend (sc), which includes the C preprocessor
* the assembler (as)
* the linker (ld)
* the preprocessor (cpp), which is not used for regular C compilation
  (bacause the one included in the frontend is used), but can be invoked
  manually
* the startup object file (crtso.o)
* the C library, libc (libc.a)
* the C include (header) files (include/*.h, include/*/*.h)

By being fully open source we mean the combination of the following:

* it is free software
* the source code is available under an OSI-approved license
* it can be compiled using itself, or a previous version of itself
* it can be bootstrapped from its source code using another C compiler (i.e.
  you don't need minixbcc, BCC, Dev86 or As86 to compile minixbcc; any K&R
  or ANSI (C89 or later) C compiler and a decent Unix-like system will do)
* the build is fully automatic, there is no need to edit any files manually
  (such as config, system config or system header files), and there is no
  need to specify any settings (because the defaults are good enough)

## How to try it

If you have a decent, modern Unix-like system (such as Linux, FreeBSD,
NetBSD, OpenBSD, DragonFly BSD, macOS, Windows WSL) with GCC (or Clang)
installed, run these commands (without the leading `$`) to compile and try
the minixbcc cross-compiler:

```
$ git clone --depth 1 https://github.com/pts/minixbcc
$ cd minixbcc
$ sh cross.sh
... (it prints thousands of progress lines)
$ bin/bbcc -0 -DPRINT_F=printf -o h0 h.c
$ bin/bbcc -3 -DPRINT_F=printf -o h3 h.c
$ ls -l h0 h3
-rwxrwxr-x 1 user group 3984 Feb  3 22:37 h0
-rwxrwxr-x 1 user group 3548 Feb  3 22:38 h3
```

If your C compiler is not called *gcc*, then specify it in the *cross.sh*
command line, e.g. `sh cross.sh clang -s -O2`. The default is `sh cross.sh
gcc -s -O2`.

Running *cross.sh* will build all the components above, more specifically:

* the cross-compiler driver: `bin/bbcc`
* the cross-compiler frontend--backend: `libexec/sc`
* the cross-compiler assembler: `libexec/as`
* the cross-compiler linker: `libexec/ld`
* the cross-compiler preprocessor: `libexec/cpp`
* the native startup object file: `0/crtso.o` and `3/crtso.o`
* the native C library: `0/libc.a` and `3/libc.a`
* the C include (header) files: `include/*.h` and `include/*/*.h`,
  no need to compile, files kept intact

If you want to build the native tools as well (which run on
Minix), use *build.sh* instead of *cross.sh*. For example: `sh build.sh gcc
-s -O2`. This builds everything above, plus the native tools:

* the native driver: `0/bbcc` and `3/bbcc`
* the native frontend--backend: `0/sc` and `3/sc`
* the native assembler: `0/as` and `3/as`
* the native linker: `0/ld` and `3/ld`
* the native preprocessor: `0/cpp` and `3/cpp`

Please note that *build.sh* (unlike *cross.sh*) runs not only on a modern
Unix-like system, but it also runs on Minix (1.5.10--2.0.4; i86 and i386),
and uses whatever C compiler (minixbcc, BCC or the ACK C compiler which
comes with Minix) it can detect.

The `0' directory name indicates the Minix 1.5.10--2.0.4 i86 target, and the
`3` directory name indicates the Minix 1.5.10--2.0.4 i386 target. Each
executable process in the *0* target is limited to 64 KiB of code, and
(independently) 64 KiB of data (including .rodata, .data, .bss, stack,
malloc(...) heap, argv strings and environ strings). The *3* target is
limited to ~2 GiB of virtual memory (code and data each), but in practice
it's ~63 MiB in total, because that's how much physical memory the Minix
2.0.4 i386 kernel supports.

The `h0` executable created above is for the *0* target, and the `h3`
executable is for the *3* target. The minixbcc tools can target any of the
*0* and the *3* targets (selectable by the command-line flags `-0` or `-3`,
respectively), no matter whether they are compiled as a cross-compiler, a
native compiler running on a *0* host, or a native compiler running on a *3*
host.

## More info about minixbcc

The most important differences from original BCC and Dev86 BCC:

* In minixbcc, signed integer division consistently rounds towards 0. This
  is consistent with C99 and the x86 idiv instruction. C89 allows either
  rounding down or rounding towards 0. BCC and Dev86 BCC behave
  inconsistently, depending on the target (i86 and i386), the type (int or
  long), the divisor (whether it's a power of 2), and the libc with which the
  compiler was compiled.
* The latest Dev86 BCC has an optimizer, which makes the assembly output of
  the C compiler frontend--backend shorter and faster. The output of the
  optimizer is still assembly, and it is fed to the assembler. minixbcc
  doesn't have an optimizer.
* Lots of bugs have been fixed in (and features have also been added to)
  Dev86 BCC between original BCC, Dev86 0.0.5 (1996-03-24) and Dev86
  0.16.21f (2015-03-08). Most of these didn't make it to minixbcc.
* The latest Dev86 BCC targets ELKS (i86), DOS (i86) and Linux (i386).
  minixbcc targets Minix i86 and Minix i386. The original BCC targets
  i86, i386 and MC6809.
* minixbcc doesn't support any operations on floating-point types (float and
  double). This is to make its code fit to 64 KiB even if compiled with
  itself, a non-optimizing compiler. Original BCC and Dev86 BCC do support
  floats.
* The C source code of minixbcc is much more portable, it can be compiled on
  most modern C or C++ compilers. Dev86 BCC can also be compiled for a DOS
  host, minixbcc can't (because the C compiler driver (cc) calls fork(3) and
  execv(3)).

Minimum system requirements for building minixbcc as a cross-compiler:

* TL;DR It will build on a modern Unix-like system with a C compiler, even
  64-bit systems.
* A C compiler (any of: traditional K&R, ANSI C (C89), C99 or newer) or
  a C++ compiler (any of: older than C++98, C++98, C++11 or newer, even
  C++23).
* Some POSIX functions in the libc. The C compiler driver (cc) needs fork(3)
  and execv(3), so it will need a Unix-like system to build and run. Windows
  users can use WSL (Windows Subsystem for Linux) or Docker.
* An ASCII-based system. (Most modern systems in 2026 are.) This is checked
  at compile time (e.g. near the beginning of [ld/ld.c](ld/ld.c)) and at
  *build.sh* build time (in [sysdet.c](sysdet.c)).
* Two's complement signed integer representation. (Most modern systems in
  2026 have it, C++20 and C23 requires it.) This is checked at compile time
  (e.g. near the beginning of [ld/ld.c](ld/ld.c)) and at
  *build.sh* build time (in [sysdet.c](sysdet.c)).
* 8-bit C char, 16-bit C short and a 32-bit C int or C long. Most modern C
  compilers in 2026 have it. This is checked at *build.sh* build time (in
  [sysdet.c](sysdet.c)).
* The size of the C pointer can be arbitrary: minixbcc
  compiles on systems with pointer size of 16-bit, 32-bit, 64-bit etc.
  Please note that if the pointer size is 16-bit, this typically means that
  memory (code, code or the total) is limited to 64 KiB. Such systems
  typically need extra porting work, because of the extremely limited
  memory. minixbcc has been ported to Minix i86 and ELKS.
  [build.sh](build.sh) autodetects a Minix 1.5.10--2.0.4 i86 host, and
  applies the version-specific workarounds needed. See the section below on
  building the cross-compiler for an ELKS host (cross-compiling it on
  Linux).

minixbcc tools and libc are written in multiple dialects of multiple
programming languages (C, C++ and x86 assembly). The tools are written in
very portable C (with a decent amount of `#ifdef`s), so they can be built
using either a traditional K&R or an ANSI C (C89), or a C99 or a newer C
compiler, or even a C++ compiler. There is no C++ code in minixbcc, just the
C source code is written in a careful way so that it also works as C++
source code. (Most of this has been achieved just by using ANSI C function
prototypes and argument type specification in function defininitons (when a
C++ compiler is detected), and using the *const* in *const char\**
consistently, as needed by C++.) Undefined behavior is avoided whenever
possible. (Most of this is done by doing unsigned integer arithmetic, which
has proper wrap-around. Also pointer arithmetic is done only on valid array
ranges.)

The minixbcc libc is written partly in x86 assembly in the minixbcc
assembler (as) syntax, and mostly in the C dialect understood by the
minixbcc C compiler (which is traditional K&R, no ANSI C; there is no
support for floating-point operations). There are no plans to port the libc
to other C compilers or to architectures other than i86 or i386.
Historically the libc was written for the ACK 3.1 C compiler running on
Minix 1.5.10 i86, but it is no longer checked whether that still works.

There is an alternative, smaller libc included, implemented in x86 assembly
in BCC (1990-06-15) assembler (as) and minixbcc assembler (as) syntax (in
files [libt0.si](libt0.si) and [libt3.si](libt3.si)) and Minix 1.5.10 ACK
3.1 assembler--linker (asld) syntax (in file [sc/lsca.s](sc/lsca.s)). This
smaller libc is used for building the tools on Minix i86 and i386. For Minix
i86, this is necessary, because some of the executable program files would
be too large (especially the code size of the C compiler frontend-backend
tool (sc)) if linked against the normal libc, or the C compiler or the
linker would run out of memory compiling the tools. There is another benefit
of this smaller libc: the tools can be built on Minix i86 and i386 without
relying on the system libc or the system C header files. This no-reliance
improves reproducibility and portability. At the most extreme, the `sh
build.sh bccbin` can build minixbcc with only 3 executable programs (sc, as
and ld) available from
[bccbin16.tar.Z](https://github.com/pts/pts-minix-1.5.10-hdd-image/releases/download/minix-1.5.10-i386-patches/bccbin16.tar.Z)
or
[bccbin32.tar.Z](https://github.com/pts/pts-minix-1.5.10-hdd-image/releases/download/minix-1.5.10-i386-patches/bccbin32.tar.Z)
(BCC, 1990-06-15).

## How to install it on Minix

To compile and install minixbcc on Minix 1.5.10--2.0.4, i86 or i386, copy
all the source files and directories to the Minix system, cd into the
directory containing *build.sh*, and run `sh build.sh`. After a few minutes
it finishes successfully, and you can run `sh install.sh` to install it. The
installed command will be `/usr/bin/bbcc`, and all the other files go to
`/usr/minixbcc/`.

Please note that the binaries (i.e. the tool executables and the libc) in
the target directories *0* and *3* are built reproducibly and
deterministically, and they will be bytewise identical, even if a different
C compiler (or cross-compiler) was used to build minixbcc, even if the host
system is of a different architecture.

## How to run the cross-compiler on very old Linux

The minixbcc cross-compiler can be built and run on very old Linux systems
the same way as on recent Linux systems: just run `sh cross.sh`.

As a specific example, [MCC
1.0](https://www.ibiblio.org/pub/historic-linux/distributions/MCC-1.0/1.0/)
is a very old distribution of Linux i386, released on 1994-05-11. It
contains Linux kernel 1.0.4, GCC 2.5.8, Bash 1.13.1, /lib/libc.so.4.5.21,
GNU Assembler 2.2, GNU linker 2.2. By default, `gcc` creates a Linux i386 a.out
ZMAGIC executable dynamically linked against /lib/libc.so.4 using /lib/ld.so.
`gcc -static` Linux i386 a.out ZMAGIC statically linked executable.

After installing the base system (from
[nocdboot](https://www.ibiblio.org/pub/historic-linux/distributions/MCC-1.0/1.0/images/nocdboot.gz)
and
[root](https://www.ibiblio.org/pub/historic-linux/distributions/MCC-1.0/1.0/images/root.gz))
to a partition of about 64 MiB, install GCC by copying
[gcca.tgz](https://www.ibiblio.org/pub/historic-linux/distributions/MCC-1.0/1.0/packages/gcca.tgz)
and
[gccb.tgz](https://www.ibiblio.org/pub/historic-linux/distributions/MCC-1.0/1.0/packages/gccb.tgz)
to the filesystem, run `/tmp/bootinstall` to install them, and then remove
the *gcca.tgz* and *gccb.tgz* files. Copy the minixbcc source directory to
the partition, and run `sh cross.sh`. (To get the native tools as well, run
`sh build.sh gcc -s -O2 -W -Wall -Werror -static` instead, or just `sh
build.sh`, to get a less optimized build.)

The cross-compiler is ready use, try compiling a hello-world program:

```
# bin/bbcc -0 -DPRINT_F=printf -o h0 h.c
# bin/bbcc -3 -DPRINT_F=printf -o h3 h.c
# ls -l h0 h3
-rwxrwxr-x 1 root root 3984 Feb  3 22:37 h0
-rwxrwxr-x 1 root root 3548 Feb  3 22:38 h3
```

## How to run the cross-compiler on ELKS

First you need a Linux i386 or amd64 system (or Windows WSL or Docker) to
cross-compile the minixbcc tools on. Then you need a bootstrap
cross-compiler which runs on Linux and targets ELKS. For ease of
installation, we'll use [ssmcc](https://gihub.com/pts/ssmcc) as the
bootstrap cross-compiler. Run these commands (without the leading `$`):

```
$ git clone --depth 1 https://github.com/pts/minixbcc
$ git clone --depth 1 https://github.com/pts/ssmcc
$ cd minixbcc
$ mkdir bin libexec
$ ../ssmcc/ssmcc -belks -Os -o libexec/sc sc/*.c
$ ../ssmcc/ssmcc -belks -Os -o libexec/as as/*.c
$ ../ssmcc/ssmcc -belks -Os -o libexec/ld ld/*.c
$ ../ssmcc/ssmcc -belks -Os -o libexec/cr cr/*.c
$ ../ssmcc/ssmcc -belks -Os -o bin/bbcc cc/*.c
$ sh cross.sh --emu ../ssmcc/tools/elksemu --libc libexec/sc libexec/as libexec/cr true
... (it prints thousands of progress lines, the last one is: + exit)
$ ls -ld include/*.h include/*/*.h 0/* 3/* libexec/* bin/* h.c
```

Then copy the files listed by the `ls -ld` command above to your ELKS
system, and run the usual test commands (without the leading `#`) on the
ELKS system to compile a hello-world program:

```
# bin/bbcc -0 -DPRINT_F=printf -o h0 h.c
# bin/bbcc -3 -DPRINT_F=printf -o h3 h.c
# ls -l h0 h3
-rwxrwxr-x 1 root root 3984 Feb  3 22:37 h0
-rwxrwxr-x 1 root root 3548 Feb  3 22:38 h3
```

Please note that this is still a cross-compiler on ELKS, it targets only
Minix (both the *0* and *3* targets are Minix-only), so it can't create a
working ELKS executable, in particular, the `h0` executable program will
will work on a Minix i86 system, but not on an ELKS system.

Alternatively, you could build libc by running `sh cross.sh --libc
libexec/sc libexec/as libexec/cr true` on ELKS, instead of the *cross.sh*
command above (run on Linix). However, the ELKS 0.8.1 /bin/sh isn't able to
run the *cross.sh* script correctly, so this will not work until the shell
is fixed.
