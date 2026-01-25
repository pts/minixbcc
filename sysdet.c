/*
 * sysdet: detect the C compiler and system properties, and also detect the operating system and the architecture based on the headers in a native executable file
 * by pts@fazekas.hu at Mon Jan 19 22:34:05 CET 2026
 */

#include <sys/types.h>  /* Minix 1.5.10 needs this before <unistd.h>. */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __STDC__
#  define CONST const
#  define P(x) x
#else
#  define CONST
#  define P(x) ()
#endif

#ifndef O_RDONLY
#  define O_RDONLY 0
#endif

#ifndef STDOUT_FILENO
#  define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#  define STDERR_FILENO 2
#endif

static void write_str P((int fd, CONST char *msg));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static void write_str(fd, msg)
int fd;
CONST char *msg;
{
  if (*msg != '\0') (void)!write(fd, msg, strlen(msg));
}

static void fatal2 P((CONST char *msg1, CONST char *msg2));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static void fatal2(msg1, msg2)
CONST char *msg1;
CONST char *msg2;
{
  write_str(STDERR_FILENO, "fatal: ");
  write_str(STDERR_FILENO, msg1);
  if (msg2) {
    write_str(STDERR_FILENO, ": ");
    write_str(STDERR_FILENO, msg2);
  }
  write_str(STDERR_FILENO, "\n");
  exit(2);
}

static char header[32];

static char osid[] = "-DOSID=? ";  /* '?', '0' for Minix i86, '3' for Minix i386, '5' for Minix-386vm or Minix-vmx. */

#undef HAVE_SIZEOF
#ifdef __SIZEOF_INT__
#  ifdef __SIZEOF_LONG__
#    ifdef __SIZEOF_POINTER__
#      define HAVE_SIZEOF 1  /* Modern GCC and Clang have it all. */
#    endif
#  endif
#endif

#define AU (((char *) 2 - (char *) 1) << (sizeof(unsigned) * 8 - 1))
#define AUL (((char *) 2 - (char *) 1) << (sizeof(unsigned long) * 2) << (sizeof(unsigned long) * 2) << (sizeof(unsigned long) * 2) << (sizeof(unsigned long) * 2 - 1))
/* Returns bool indicating whether pointer arithmetics is linear. True on
 * most systems with a flat memory model. True on DOS for the small and
 * medium memory models, false for the large, compact and huge memory
 * models.
 */
int alignptrcheck P((char *cp));
int alignptrcheck(cp)
char *cp;
{
#ifdef ALIGNPTRCHECK
  (void)cp;

  return ALIGNPTRCHECK;
#else
#ifdef HAVE_SIZEOF
#if __SIZEOF_POINTER__ == __SIZEOF_INT__
  return (char *) ((unsigned) cp + AU) != cp + AU;
#else
#if __SIZEOF_POINTER__ == __SIZEOF_LONG__
  return (char *) ((unsigned long) cp + AUL) != cp + AUL;
#else
  return 0;
#endif
#endif
#else
#if 0  /* This implementation would trigger the GCC warnings -Wpointer-to-int-cast -Wint-to-pointer-cast because of the size mismatch between the pointer and the integer. */
  return sizeof(char *) == sizeof(int) ? ((char *) ((unsigned) cp + AU) != cp + AU) :
      sizeof(char *) == sizeof(long) ? ((char *) ((unsigned long) cp + AUL) != cp + AUL) : 0;
#else  /* Longer implementation, but doesn't trigger warnings. */
  union { unsigned ui; unsigned long ul; char *cp; } u;
  u.cp = cp;
  if (sizeof(char *) == sizeof(int)) {
    u.ui += AU;
    return u.cp != cp + AU;
  } else if (sizeof(char *) == sizeof(long)) {
    u.ul += AUL;
    return u.cp != cp + AUL;
  } else {
    return 0;  /* The result is irrelevant in this case. */
  }
#endif
#endif
#endif
}

int main P((int argc, char **argv));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
int main(argc, argv)
int argc;
char **argv;
{
  int fd;
  register int got;
  char *p;
  CONST char *filename;
  unsigned u;

  (void)argc;

  /* Pacify OpenWatcom v2 (owcc -W -Wall -Werror) warning `W201: Unreachable code' int the lines defining the labels. None of the conditions below are true. */
  if (argc < -5) goto ur4;
  if (argc < -4) goto ur5;
  if (argc < -3) goto ur3;
  if (argc < -2) goto ur2;
  if (argc < -1) goto ur1;

  if ((int) ~(unsigned) 0 != -1 || (-1 & 3) != 3) ur1: write_str(STDOUT_FILENO, "-DBADSIGNED ");  /* int is not two's complement. */
  u = (unsigned) 1 << (sizeof(unsigned) * 4);  /* Separate assignment to avoid warning in Minix 1.5.10 i86 ACK 3.1 C compiler: overflow in unsigned constant expression. */
  if (((unsigned) 1 << (sizeof(unsigned) * 8 - 1)) == 0 || (u << (sizeof(unsigned) * 4))) ur2: write_str(STDOUT_FILENO, "-DBADBYTE ");  /* 1 byte is not 8 bits. */
  if (sizeof(char) != 1) ur3: write_str(STDOUT_FILENO, "-DBADCHAR ");  /* char is not 1 byte. */
  if (sizeof(short) != 2) ur4: write_str(STDOUT_FILENO, "-DBADSHORT ");  /* short is not 2 bytes. The C standard allows 2 or more. */
  if (sizeof(int) != 2 && sizeof(int) != 4) ur5: write_str(STDOUT_FILENO, "-DBADINT ");  /* int is not 2 or 4 bytes. The C standard allows 2 or more. */
  write_str(STDOUT_FILENO,
      sizeof(int ) == 4 ? "-DINT32T=int " :
      sizeof(long) == 4 ? "-DINT32T=long " :
      "-DBADLONG ");  /* Neither int nor long is 4 bytes. */
#if 0
  gp += 1;  /* Even this pointer arithmetics makes qemu-ppc crash with a CPU trap. */
#endif
  write_str(STDOUT_FILENO,
      sizeof(char *) == sizeof(int ) ? "-DINTPTRT=int " :
      sizeof(char *) == sizeof(long) ? "-DINTPTRT=long " :  /* (char *) be even 8 bytes, e.g. on amd64. GCC has sizeof(long) == 8 also there. !! Add `long long' as an option? Then #define longlong long long */
      "-DBADPTR ");  /* Size of a pointer is not the same as size of int or long */
  write_str(STDOUT_FILENO,
      (sizeof(char *) & 0x1f) == 0 ? "-DBADALIGN " :  /* It could be "-DALIGNBYTES=32 " etc., but in practice it never happens. */
      (sizeof(char *) & 0xf) == 0 ? "-DALIGNBYTES=16 " :  /* ALIGNBYTES is always a power of 2. */
      (sizeof(char *) & 7) == 0 ? "-DALIGNBYTES=8 " : "-DALIGNBYTES=4 ");
  /* Some memory models (such as the huge, large and compact models on DOS)
   * have a pointer consisting of a segment:offset pair, and in these models
   * it's incorrect to cast a pointer to an integer, do arithmetic and cast
   * it back. Our non-portable alignment macro (in sc/align.h and
   * ld/align.h) does exactly this, so we want to use the portable one
   * (directed by -DPORTALIGN) instead.
   *
   * For the check we don't use (char *) 0, because pointer arithmetic on a
   * NULL poiner is undefined behavior in C, and e.g. Clang 15.0.7 with `-O0
   * -fno-wrapv' (both are defaults) would insert a check and a trap
   * instruction. We don't use `(char *) argc' either, to avoid the GCC and
   * Clang warning -Wint-to-pointer-cast.
   */
  write_str(STDOUT_FILENO, alignptrcheck((char *) 1) ? "-DNOPORTALIGN=0 " : "-DNOPORTALIGN=1 ");

  if (argv[0] && (filename = argv[1])) {
    if (argv[2]) fatal2("too many command-line arguments", (CONST char*)0);
    if ((fd = open(filename, O_RDONLY)) < 0) fatal2("error opening executable file", filename);
    for (p = header; p < header + sizeof(header); ) {
      if ((got = read(fd, p, header + sizeof(header) - p)) < 0) fatal2("error reading header", filename);
      if (got == 0) {
        for (p = header; p < header + sizeof(header); *p++ = '\0') {}
        break;
      }
      p += got;
    }
    /* close(afd): */  /* Not needed, the kernel will close it when the process exits. */
    if (header[0] == 1 && header[1] == 3 &&  /* a.out a_magic. Minix or ELKS executable. */
        ((got = header[2] & (0xff - 3)) == 0x20 || got == 0x10 || got == 0) &&  /* a_flags. 0x10 == A_EXEC (I&D in same address space ; 0x20 == A_SEP (separate I&D) */
        ((got = header[2] & 3) == 0 || got == 3) &&   /* A_UZP (== 1, unmapped zero page) | A_PAL (== 2, page-aligned executable) */
        (header[3] == 4 || header[3] == 0x10) &&  /* a_cpu. Architecture (CPU): 4 for i86 (8086+ in 16-bit mode: real mode on 16-bit protected mode), 0x10 for i386 (32-bit protected mode). */
        header[4] == 0x20 &&  /* a_hdrlen. a.out object files may have this 0x30. */
        header[5] == 0 &&  /* a_unused. */
        header[6] == 0 && header[7] == 0) {  /* a_version. */
      osid[sizeof(osid) - 3] = (header[3] == 4) ?
          (((header[2] & 3) == 0 && (header[2] & (0xff - 3)) != 0) ? '0' : '?'):  /* '0' is Minix i86. '?' is unknown. */  /* TODO(pts): Distinguish this from ELKS (i86) executable, and detect '4' (ELKS). Only some ELKS executables have a_version == 1. */
          (((header[2] & 3) == 3) ? '5' : '3');  /* '5' is Minix-386vm or Minix-vmx. '3' is Minix i386. */  /* The ACK ANSI C compiler 1.202 in Minix 1.7.0 i386 sets a_flags to 0 (not even A_EXEC or A_SEP. We allow this here, but not for i86. */
    }
    if (osid[sizeof(osid) - 3] != '?') write_str(STDOUT_FILENO, osid);
  }
  write_str(STDOUT_FILENO, "\n");
  return 0;
}
